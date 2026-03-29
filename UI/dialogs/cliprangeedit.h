#ifndef CLIPRANGEEDIT_H
#define CLIPRANGEEDIT_H
#include "UI/framelessdialog.h"
#include <QAbstractItemModel>
#include "Play/Danmu/common.h"


class ClipRangeEdit : public CFramelessDialog
{
    Q_OBJECT
public:
    ClipRangeEdit(const DanmuSource *src, QVector<SimpleDanmuInfo> *dmList, QWidget *parent = nullptr, int duration = -1);
    int clipStart, clipDuration;
};

class ClipDanmuPoolModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ClipDanmuPoolModel(const QVector<SimpleDanmuInfo> *dmList, QObject *parent=nullptr);
    QModelIndex getIndex(int time);
    void setClipRange(int start, int end);
private:
    const QVector<SimpleDanmuInfo> *m_dmList;
    int clipStart, clipEnd;
    // QAbstractItemModel interface
public:
    inline virtual QModelIndex index(int row, int column, const QModelIndex &parent) const{return parent.isValid()?QModelIndex():createIndex(row,column);}
    inline virtual QModelIndex parent(const QModelIndex &) const{return QModelIndex();}
    inline virtual int rowCount(const QModelIndex &parent) const{return parent.isValid() ? 0 : m_dmList->count();}
    inline virtual int columnCount(const QModelIndex &parent) const {return parent.isValid() ? 0 : 2;}
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};
#endif // CLIPRANGEEDIT_H
