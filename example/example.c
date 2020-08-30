//Including lua headers
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
//Including luaThreading header
#include <luaThreading.h>

int main(){
    //Generating a lua state the usual way
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    //Making the luaThreading functions available from the created lua state
    lt_include(L);
    //Calling the main function from example.lua
    luaL_dofile(L, "example.lua");
    lua_getglobal(L, "main");
    lua_call(L, 0, 0);
    //Cleaning up
    lua_close(L);
    return 0;
}

