#include "luaStateGen.h"

stateGenerator* lsg_init(int numberOfFiles, const char** files, include_fnc function){
    stateGenerator* ret = malloc(sizeof(struct stateGenerator_struct));
    ret->numberOfFiles = numberOfFiles;
    ret->files = files;
    ret->function = function;
    return ret;
}

#include <stdio.h>
#include <inttypes.h>
lua_State* lsg_makeState(stateGenerator* sg){
    lua_State* ret;
    ret = luaL_newstate();
    luaL_openlibs(ret);
    for(int i=0; i<sg->numberOfFiles; i++)
        luaL_dofile(ret, sg->files[i]);
    include_fnc fnc;
    fnc = sg->function;
    fnc(ret);
    lua_pushinteger(ret, (uint64_t) sg);
    lua_setglobal(ret, "STATE_GENERATOR");
    return ret;
}

lua_State* lsg_regenState(lua_State* L){
    lua_getglobal(L, "STATE_GENERATOR");
    uint64_t addr = lua_tointeger(L,-1);
    stateGenerator* sg = (stateGenerator*) addr;
    return lsg_makeState(sg);
}

