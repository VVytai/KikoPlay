#include "dbmanager.h"
#include "globalobjects.h"
#include <QSqlQuery>
#include <QSqlError>
#include "logger.h"

DBManager::DBManager()
{
    for (int i = 0; i < DBType::UNKNOWN; ++i)
    {
        QString fn(GlobalObjects::context()->dataPath + dbFiles[i] + ".db");
        bool needInit = !QFile::exists(fn);
        QSqlDatabase db = getDB(DBType(i));
        if (needInit)
        {
            initDB(db, DBType(i));
        }
        checkUpdateDB(db, DBType(i));
    }
}

DBManager *DBManager::instance()
{
    static DBManager manager;
    return &manager;
}

QSqlDatabase DBManager::getDB(DBType db)
{
    if (!dbManagers[db].hasLocalData())
    {
        dbManagers[db].setLocalData(QSharedPointer<DBConn>::create(openDBConn(db)));
    }
    return dbManagers[db].localData().get()->db;
}

QSqlDatabase DBManager::openDBConn(DBType db)
{
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE", QString("%1-%2").arg(dbFiles[db]).arg(reinterpret_cast<int64_t>(QThread::currentThreadId())));
    QString dbFile(GlobalObjects::context()->dataPath + dbFiles[db] + ".db");
    database.setDatabaseName(dbFile);
    if (!database.open())
    {
        Logger::logger()->log(Logger::APP, database.lastError().text());
        return database;
    }
    QSqlQuery query(database);
    query.exec("PRAGMA foreign_keys = ON;");
    return database;
}

void DBManager::initDB(QSqlDatabase &db, DBType type)
{
    if (type == DBType::UNKNOWN) return;
    QFile sqlFile(QString(":/res/db/%1.sql").arg(dbFiles[static_cast<int>(type)]));
    if (sqlFile.open(QFile::ReadOnly))
    {
        QSqlQuery query(db);
        QStringList sqls = QString(sqlFile.readAll()).split(';', Qt::SkipEmptyParts);
        for (const QString &sql : sqls)
        {
            query.exec(sql);
        }
        query.exec(QString("PRAGMA user_version = %1;").arg(GlobalObjects::kikoVersionNum));
    }
}

void DBManager::checkUpdateDB(QSqlDatabase &db, DBType type)
{
    if (type == DBType::UNKNOWN) return;
    QFile updateFile(QString(":/res/db/%1_update.sql").arg(dbFiles[static_cast<int>(type)]));
    if (!updateFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QSqlQuery query(db);
    query.exec("PRAGMA user_version;");
    int currentVersion = 200000;
    if (query.next())
    {
        currentVersion = query.value(0).toInt();
    }
    Logger::logger()->log(Logger::APP, QString("[checkdb]db: %1, version: %2").arg(dbFiles[static_cast<int>(type)]).arg(currentVersion));
    if (currentVersion >= GlobalObjects::kikoVersionNum) return;

    QTextStream in(&updateFile);
    QString currentSql;
    int parsingVersion = 0;
    auto executeSqlScript = [&db](const QString &sql){
        QStringList sqls = sql.split(';', Qt::SkipEmptyParts);
        if (sqls.empty()) return;
        QSqlQuery query(db);
        for (const QString &sql : sqls)
        {
            if (sql.trimmed().isEmpty()) continue;
            query.exec(sql);
        }
    };

    db.transaction();

    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.startsWith("-- @VERSION:"))
        {
            if (parsingVersion > currentVersion)
            {
                executeSqlScript(currentSql);
                QSqlQuery(db).exec(QString("PRAGMA user_version = %1;").arg(parsingVersion));
                Logger::logger()->log(Logger::APP, QString("[checkdb]update db(%1) to version: %2").arg(dbFiles[static_cast<int>(type)]).arg(parsingVersion));
            }
            parsingVersion = line.mid(12).trimmed().toInt();
            currentSql.clear();
        }
        else if (!line.isEmpty() && !line.startsWith("--"))
        {
            currentSql += line + "\n";
        }
    }

    if (parsingVersion > currentVersion || currentVersion == 0)
    {
        executeSqlScript(currentSql);
        QSqlQuery(db).exec(QString("PRAGMA user_version = %1;").arg(parsingVersion == 0 ? GlobalObjects::kikoVersionNum : parsingVersion));
        Logger::logger()->log(Logger::APP, QString("[checkdb]update db(%1) to version: %2").arg(dbFiles[static_cast<int>(type)]).arg(parsingVersion));
    }

    db.commit();
}
