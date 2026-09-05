#ifndef EXT_COMMON_H
#define EXT_COMMON_H


#include "Extension/Lua/lua.hpp"
#include <QVariant>

namespace Extension
{

class LuaStackGuard
{
public:
    explicit LuaStackGuard(lua_State *state) : L(state), restoreTop(state ? lua_gettop(state) : 0) {}
    LuaStackGuard(lua_State *state, int top) : L(state), restoreTop(top) {}
    ~LuaStackGuard()
    {
        if (L) lua_settop(L, restoreTop);
    }

    LuaStackGuard(const LuaStackGuard &) = delete;
    LuaStackGuard &operator=(const LuaStackGuard &) = delete;

private:
    lua_State *L;
    int restoreTop;
};

inline bool isValidLuaRef(int ref)
{
    return ref != LUA_NOREF && ref != LUA_REFNIL;
}

inline bool pushLuaTableCallback(lua_State *L, int tableRef, const char *name)
{
    if (!L || !isValidLuaRef(tableRef) || !lua_checkstack(L, 2)) return false;
    if (lua_rawgeti(L, LUA_REGISTRYINDEX, tableRef) != LUA_TTABLE)
    {
        lua_pop(L, 1);
        return false;
    }
    lua_getfield(L, -1, name);
    lua_remove(L, -2);
    if (!lua_isfunction(L, -1))
    {
        lua_pop(L, 1);
        return false;
    }
    return true;
}

inline bool pushLuaUserdataCallback(lua_State *L, int userdataRef, const char *name)
{
    if (!L || !isValidLuaRef(userdataRef) || !lua_checkstack(L, 3)) return false;
    if (lua_rawgeti(L, LUA_REGISTRYINDEX, userdataRef) != LUA_TUSERDATA)
    {
        lua_pop(L, 1);
        return false;
    }
    lua_getuservalue(L, -1);
    lua_remove(L, -2);
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return false;
    }
    lua_getfield(L, -1, name);
    lua_remove(L, -2);
    if (!lua_isfunction(L, -1))
    {
        lua_pop(L, 1);
        return false;
    }
    return true;
}

struct AppRes
{
    AppRes() = default;
    AppRes &operator=(const AppRes&) = delete;
    AppRes(const AppRes&) = delete;
    virtual ~AppRes() {}
};
struct LuaItemRef
{
    int ref = -1;
    int tableRef = -1;
};


typedef std::function<void (lua_State *, QVariantMap &, int)> MapModifier;

int getTableLength(lua_State *L, int pos);
void pushValue(lua_State *L, const QVariant &val);
QVariant getValue(lua_State *L, bool useString, int parseLevel = -1, int curLevel = 0);
QVariant getValue(lua_State *L, bool useString, const MapModifier &modifier, int parseLevel = -1, int curLevel = 0);

}
Q_DECLARE_METATYPE(Extension::LuaItemRef)

#endif // EXT_COMMON_H
