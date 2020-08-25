#ifndef LUATHREADING
#define LUATHREADING
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include "luaStateGen.h"

struct lt_threaded_args{
    lua_State* state;
    const char* func;
};

int lt_runFunc(lua_State* L);
void* lt_threaded(void* args);
void lt_include(lua_State* L);


#endif

