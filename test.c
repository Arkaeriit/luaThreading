#include "luaThreading.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int main(){
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    lt_include(L);
    luaL_dofile(L, "exp.lua");
    lua_getglobal(L, "main");
    lua_call(L, 0, 0);
    lua_close(L);
    return 0;
}

