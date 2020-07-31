#ifndef LUASTATEGEN
#define LUASTATEGEN
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>

typedef struct stateGenerator_struct {
    int numberOfFiles,
    const char** files, //Use of const in order not to wory about freeing or anything
    int numberOfFunc,
    const void* functions,
} stateGenerator;

stateGenerator* lsg_init(int numberOfFiles, const char** files, int numberOfFunc, const void* functions);
lua_State* lsg_make_state(stateGenerator* sg);

#endif

