#ifndef LUATHREADING
#define LUATHREADING
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>

struct lt_threaded_thread{
    lua_State* state;
    const char* func;
    pthread_t* thread;
};

int lt_runFunc(lua_State* L);
void* lt_threaded(void* args);
int lt_closeThread(lua_State* L);
void lt_include(lua_State* L);
void lt_swapElem(lua_State* from, lua_State* to);


#endif

