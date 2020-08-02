#ifndef LUATHREADING
#define LUATHREADING
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <stdint.h>
#include "luaStateGen.h"

int lt_runFunc(luaState* L);



#endif

