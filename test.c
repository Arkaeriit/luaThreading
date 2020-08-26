#include "luaThreading.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int main(){
    const char* files = "exp.lua";
    stateGenerator* sg = lsg_init(1, &files, lt_include);
    lua_State* L = lsg_makeState(sg);
    lua_getglobal(L, "main");
    lua_call(L, 0, 0);
    lsg_purge(L);
    return 0;
}

