#include "luaThreading.h"

/*
 * Used to create the createThread lua function which take the name of
 * a function as argument a return a thread object
 * WIP
 */
int lt_runFunc(lua_State* L){
    const char* func = luaL_checkstring(L, 1);
    lua_State* copy = lua_newthread(L);
    //Creating arguments
    struct lt_threaded_thread* lt_thread =  malloc(sizeof(struct lt_threaded_thread));
    lt_thread->state = copy; 
    lt_thread->func = func;
    //Creating thread
    pthread_t* thread = malloc(sizeof(pthread_t));
	pthread_create(thread, NULL, lt_threaded, (void *) lt_thread);
    //Returnung the thread
    lt_thread->thread = thread;
    lua_pushinteger(L, (uint64_t) lt_thread);
    return 1;
}

/*
 * Run a function from a lua_State in a new thread
 *  Argument:
 *      args : a pointer to a lt_threaded_thread struct, contain a
 *             lua_State pointer, the name of the function to run
 *             and will be added a pointer to the thread where the
 *             lua_State is running
 *  return:
 *      NULL
 */
void* lt_threaded(void* args){
    struct lt_threaded_thread* lt_args = args;
    lua_getglobal(lt_args->state, lt_args->func);
    lua_call(lt_args->state, 0, 0);
    return NULL;
}    

/*
 * Used to create the joinThread lua function. Take a thread as an
 * argument, join it and close the lua_State it ran on.
 * WIP
 */
int lt_closeThread(lua_State* L){
    struct lt_threaded_thread* lt_thread = (struct lt_threaded_thread*) lua_tointeger(L, 1);
    //Closing the thread
    pthread_join(*(lt_thread->thread), NULL);
    //Freeing everyone
    free(lt_thread->thread);
    free(lt_thread);
    return 0;
}

void lt_include(lua_State* L){
    lua_pushcfunction(L, lt_runFunc);
    lua_setglobal(L, "createThread");
    lua_pushcfunction(L, lt_closeThread);
    lua_setglobal(L, "joinThread");
}

