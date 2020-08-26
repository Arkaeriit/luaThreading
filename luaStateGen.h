#ifndef LUASTATEGEN
#define LUASTATEGEN
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <inttypes.h>

typedef void (*include_fnc)(lua_State* L);

typedef struct stateGenerator_struct {
    int numberOfFiles;
    const char** files; //Use of const in order not to wory about freeing or anything
    include_fnc function;
} stateGenerator;

stateGenerator* lsg_init(int numberOfFiles, const char** files, include_fnc function);
lua_State* lsg_makeState(stateGenerator* sg);
lua_State* lsg_regenState(lua_State* L);
void lsg_purge(lua_State* L);

#endif

