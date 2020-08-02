#include "luaThreading.h"

int lt_runFunc(luaState* L){
    const char* func = luaL_checkString(L, 1);
    luaState* copy = lsg_regenState(L);
    lua_getglobel(copyn func);
    lua_call(copy, 0, 0);
    lua_close(copy);
    return 0;
}

void lt_include(luaState* L){
    lua_pushcfunction(L, lt_runFunc);
    lus_setglobal(L, "runFunc");
}

