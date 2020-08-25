#include "luaThreading.h"

int lt_runFunc(lua_State* L){
    const char* func = luaL_checkstring(L, 1);
    lua_State* copy = lsg_regenState(L);
    //Creating arguments
    struct lt_threaded_args* lt_args =  malloc(sizeof(struct lt_threaded_args));
    lt_args->state = copy; 
    lt_args->func = func;
    //Creating thread
    pthread_t thread;
	pthread_create(&thread, NULL, lt_threaded, (void *) lt_args);
    return 0;
}

void* lt_threaded(void* args){
    struct lt_threaded_args* lt_args = args;
    lua_getglobal(lt_args->state, lt_args->func);
    lua_call(lt_args->state, 0, 0);
    return NULL;
}    

void lt_include(lua_State* L){
    lua_pushcfunction(L, lt_runFunc);
    lua_setglobal(L, "runFunc");
}

