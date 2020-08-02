#ifndef LUATHREADING
#define LUATHREADING
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <stdint.h>
#include "luaStateGen.h"

int lt_runFunc(lua_State* L);

void lt_include(lua_State* L);


#endif

