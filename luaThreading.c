//Includes
#include "luaThreading.h"
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>

#define REGISTRY_TABLE_NAME    "LUA_THREADING_GLOBAL_CONTEXT"
#define FUNCTION_TABLE_FIELD "__function_table"
#define MUTEX_FIELD          "__mutex"

//Types definitions
struct lt_threaded_thread{
    lua_State* state;
    pthread_t* thread;
    int number_of_arguments;
    int function_id; // Id of the function to call in the global list
};

//Static functions definitions
static int lt_runFunc(lua_State* L);
static void* lt_threaded(void* args);
static int lt_closeThread(lua_State* L);
static void lt_swapElem(lua_State* from, int index, lua_State* to);

//Function code

/*
 * Used to create the _launchThread function which run
 * LUATHREAD_GLOBAL_NAME.LUATHREAD_INTERNAL_FUNCTION in a new thread
 */
static int lt_runFunc(lua_State* L){
    int num_args = lua_gettop(L);
    lua_createtable(L, 0, 3);
    lua_State* copy = lua_newthread(L);
    lua_setfield(L, -2, "state");
    struct lt_threaded_thread* lt_thread =  malloc(sizeof(struct lt_threaded_thread));
    lt_thread->state = copy; 
    lt_thread->number_of_arguments = num_args-1;

    // Preparing the return value. This value is a table that contains the
    // fields:
    //  * ctx: the lt_threaded_thread struct as a lightuserdata
    //  * state: the lua state generated by newthread, stored to prevent gc
    //  * function: the called function, stored to prevent gc

    // Puting function in return value
    lua_pushvalue(L, 1);
    lua_setfield(L, -2, "function");

    // Puting function in the global function table
    lua_getfield(L, LUA_REGISTRYINDEX, REGISTRY_TABLE_NAME);
    lua_getfield(L, -1, MUTEX_FIELD);
    pthread_mutex_t* mutex = lua_touserdata(L, -1);
    lua_pop(L, 1);
    pthread_mutex_lock(mutex);
    lua_getfield(L, -1, FUNCTION_TABLE_FIELD);
    lua_len(L, -1);
    int ftable_len = lua_tointeger(L, -1);
    lua_pushvalue(L, 1);
    lua_seti(L, -3, ftable_len+1);
    lua_pop(L, 3);
    pthread_mutex_unlock(mutex);
    lt_thread->function_id = ftable_len+1; // TODO: gc the leftover functions

    // Reading the function from the global table and pushing it on the stack
    // of the thread
    pthread_mutex_lock(mutex);
    lua_getfield(copy, LUA_REGISTRYINDEX, REGISTRY_TABLE_NAME);
    lua_getfield(copy, -1, FUNCTION_TABLE_FIELD);
    lua_geti(copy, -1, lt_thread->function_id);
    pthread_mutex_unlock(mutex);

    //Creating arguments
    for (int i=0; i<num_args-1; i++) {
        lt_swapElem(L, 2+i, copy);
    }
    //Creating thread
    pthread_t* thread = malloc(sizeof(pthread_t));
	pthread_create(thread, NULL, lt_threaded, (void *) lt_thread);
    //Returnung the thread
    lt_thread->thread = thread;
    lua_pushlightuserdata(L, lt_thread);
    lua_setfield(L, -2, "ctx");
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
static void* lt_threaded(void* args){
    struct lt_threaded_thread* lt_args = args;
    lua_call(lt_args->state, lt_args->number_of_arguments, 1);
    return NULL;
}    

/*
 * Used to create the joinThread lua function. Take a thread as an
 * argument, join it and close the lua_State it ran on.
 * Return the result of the function callded when lauching the thread.
 */
static int lt_closeThread(lua_State* L){
    lua_getfield(L, -1, "ctx");
    struct lt_threaded_thread* lt_thread = lua_touserdata(L, -1);
    //Closing the thread
    pthread_join(*(lt_thread->thread), NULL);
    //Puting the result value
    lt_swapElem(lt_thread->state, -1, L);
    //Freeing everyone
    free(lt_thread->thread);
    free(lt_thread);
    return 1;
}

/*
 * Push an element of a lua_State to the to of an other lua_State
 *  Arguments:
 *      from : The state we take an element from
 *      index : the position of the element in from
 *      to : the state we put the element on
 *  error:
 *      If the type on the element on the stack can not be determined,
 *      an error message is printed, the program continues with a nil
 *      pushed on the receiving stack
 */
static void lt_swapElem(lua_State* from, int index, lua_State* to){
    int type = lua_type(from, index);
    /*fprintf(stdout, "types : %i; nil : %i, is nill : %i\n",type, LUA_TNIL, type == LUA_TNIL);*/
    switch(type){
        case LUA_TNIL:
            lua_pushnil(to);
            break;
        case LUA_TNUMBER:
            if(lua_isinteger(from, index)){
                lua_Integer i = lua_tointeger(from, index);
                lua_pushinteger(to, i);
            }else{
                lua_Number n = lua_tonumber(from, index);
                lua_pushnumber(to, n);
            }
            break;
        case LUA_TBOOLEAN:
            lua_pushboolean(to, lua_toboolean(from, index));
            break;
        case LUA_TSTRING:
            lua_pushstring(to, lua_tostring(from, index));
            break;
        case LUA_TUSERDATA:
        case LUA_TLIGHTUSERDATA:
            lua_pushlightuserdata(to, lua_touserdata(from, index));
            break;
        /*case LUA_TTHREAD: //Invalid: must find a good way to do so
            lua_pushthread(to, lua_tothread(from, index));
            break;*/
        case LUA_TFUNCTION:
            if(lua_iscfunction(from, index)){
                lua_CFunction func = lua_tocfunction(from, index);
                lua_pushcfunction(to, func);
                break;
            } else {
                printf("Hmmm...");
            }
        /*case LUA_TTABLE: // TODO
            lua_getglobal(from, "LUATHREAD_TABLE_DETAIL");
            lua_insert(from, -2);
            lua_call(from, 1, 1);
            lua_geti(from, -1, 0);
            int numberOfKeys = lua_tointeger(from, -1);
            lua_pop(from, 1);
            lua_createtable (to, numberOfKeys, numberOfKeys);
            for(int i=1; i<=numberOfKeys; i++){
                lua_geti(from, -1, i);
                //putting the key in the receiving table
                lua_pushstring(from, "key");
                lua_gettable(from, -2); 
                lt_swapElem(from, to);
                //putting the value
                lua_pushstring(from, "value");
                lua_gettable(from, -3); 
                lt_swapElem(from, to);
                //associating and cleaning
                lua_settable(to, -3);
                lua_pop(from, 3);
            }
            break;*/
        default:
            fprintf(stderr, "Error: unknow or invalid type on top of a stack.\n");
    }
}

static const struct luaL_Reg luaThreading [] = {
    {"launchThread", lt_runFunc},
    {"joinThread", lt_closeThread},
    {NULL, NULL} /* sentinel */
};

static void manage_global_context(lua_State* L) {
    if (lua_getfield(L, LUA_REGISTRYINDEX, REGISTRY_TABLE_NAME) == LUA_TTABLE) {
        // Context already generated, no need to do it again
        lua_pop(L, 1);
        return;
    }
    lua_pop(L, 1);
    // TODO: meta-table to garbage collect with __gc
    lua_createtable(L, 0, 2);
    // Makes a table that is used to store functions to call in thread.
    lua_createtable(L, 10, 0);
    lua_setfield(L, -2, FUNCTION_TABLE_FIELD);
    // As that table might be used in multiple threads, we want to protect
    // it with a mutex.
    pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex, NULL);
    lua_pushlightuserdata(L, mutex);
    lua_setfield(L, -2, MUTEX_FIELD);
    lua_setfield(L, LUA_REGISTRYINDEX, REGISTRY_TABLE_NAME);
}

int luaopen_luaThreading(lua_State* L) {
    luaL_newlib(L, luaThreading);
    manage_global_context(L);
    return 1;
}

