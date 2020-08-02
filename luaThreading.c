#include "luaThreading.h"

int lt_runFunc(lua_State* L){
    const char* func = luaL_checkstring(L, 1);
    lua_State* copy = lsg_regenState(L);
    lua_getglobal(copy, func);
    lua_call(copy, 0, 0);
    lua_close(copy);
    return 0;
}

void lt_include(lua_State* L){
    lua_pushcfunction(L, lt_runFunc);
    lua_setglobal(L, "runFunc");
}

