//Includes
#include "luaThreading.h"
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>

//Types definitions
struct lt_threaded_thread{
    lua_State* state;
    pthread_t* thread;
};

//Static functions definitions
static int lt_runFunc(lua_State* L);
static void* lt_threaded(void* args);
static int lt_closeThread(lua_State* L);
static void lt_swapElem(lua_State* from, lua_State* to);

//Function code

/*
 * Used to create the _launchThread function which run
 * LUATHREAD_GLOBAL_NAME.LUATHREAD_INTERNAL_FUNCTION in a new thread
 */
static int lt_runFunc(lua_State* L){
    lua_State* copy = lua_newthread(L);
    //Creating arguments
    struct lt_threaded_thread* lt_thread =  malloc(sizeof(struct lt_threaded_thread));
    lt_thread->state = copy; 
    //Creating thread
    pthread_t* thread = malloc(sizeof(pthread_t));
	pthread_create(thread, NULL, lt_threaded, (void *) lt_thread);
    //Returnung the thread
    lt_thread->thread = thread;
    lua_pushlightuserdata(L, lt_thread);
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
    lua_getglobal(lt_args->state, "LUATHREAD_GLOBAL_NAME");
    lua_pushstring(lt_args->state, "LUATHREAD_INTERNAL_FUNCTION");
    lua_gettable(lt_args->state, -2);
    lua_call(lt_args->state, 0, 1);
    return NULL;
}    

/*
 * Used to create the joinThread lua function. Take a thread as an
 * argument, join it and close the lua_State it ran on.
 * Return the result of the function callded when lauching the thread.
 */
static int lt_closeThread(lua_State* L){
    struct lt_threaded_thread* lt_thread = lua_touserdata(L, -1);
    //Closing the thread
    pthread_join(*(lt_thread->thread), NULL);
    //Puting the result value
    lt_swapElem(lt_thread->state, L);
    //Freeing everyone
    free(lt_thread->thread);
    free(lt_thread);
    return 1;
}

/*
 * Push the element at the top of a lua_State to an other lua_State
 *  Arguments:
 *      from : The state we take an element from
 *      to : the state we put the element on
 *  error:
 *      If the type on the element on the stack can not be determined,
 *      an error message is printed, the program continues with a nil
 *      pushed on the receiving stack
 */
static void lt_swapElem(lua_State* from, lua_State* to){
    int type = lua_type(from, -1);
    /*fprintf(stdout, "types : %i; nil : %i, is nill : %i\n",type, LUA_TNIL, type == LUA_TNIL);*/
    switch(type){
        case LUA_TNIL:
            lua_pushnil(to);
            break;
        case LUA_TNUMBER:
            if(lua_isinteger(from, -1)){
                lua_Integer i = lua_tointeger(from, -1);
                lua_pushinteger(to, i);
            }else{
                lua_Number n = lua_tonumber(from, -1);
                lua_pushnumber(to, n);
            }
            break;
        case LUA_TBOOLEAN:
            lua_pushboolean(to, lua_toboolean(from, -1));
            break;
        case LUA_TSTRING:
            lua_pushstring(to, lua_tostring(from, -1));
            break;
        case LUA_TUSERDATA:
        case LUA_TLIGHTUSERDATA:
            lua_pushlightuserdata(to, lua_touserdata(from, -1));
            break;
        /*case LUA_TTHREAD: //Invalid: must find a good way to do so
            lua_pushthread(to, lua_tothread(from, -1));
            break;*/
        case LUA_TFUNCTION:
            if(lua_iscfunction(from, -1)){
                lua_CFunction func = lua_tocfunction(from, -1);
                lua_pushcfunction(to, func);
            }
            break;
        case LUA_TTABLE:
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
            break;
        default:
            fprintf(stderr, "Error: unknow or invalid type on top of a stack.\n");
    }
}

/*
 * Include the librairy under the global name libName
 */
void lt_include(lua_State* L, const char* libName){
    lua_createtable(L, 0, 3);
    lua_setglobal(L, "LUATHREAD_GLOBAL_NAME");
    luaL_dofile(L, "/usr/local/lib/luaThreading/luaThreading.luac");
    luaL_dofile(L, "/usr/local/lib64/luaThreading/luaThreading.luac");
    luaL_dofile(L, "/usr/lib/luaThreading/luaThreading.luac");
    luaL_dofile(L, "/usr/lib64/luaThreading/luaThreading.luac");
    luaL_dofile(L, "luaThreading.lua");
    lua_getglobal(L, "LUATHREAD_GLOBAL_NAME");
    lua_pushstring(L, "_launchThread");
    lua_pushcfunction(L, lt_runFunc);
    lua_settable(L, -3);
    lua_pushstring(L, "joinThread");
    lua_pushcfunction(L, lt_closeThread);
    lua_settable(L, -3);
    lua_setglobal(L, libName);
}

static const struct luaL_Reg luaThreading [] = {
    {"_launchThread", lt_runFunc},
    {"joinThread", lt_closeThread},
    {NULL, NULL} /* sentinel */
};

int luaopen_luaThreading(lua_State* L){
    luaL_newlib(L, luaThreading);
    lua_setglobal(L, "LUATHREAD_GLOBAL_NAME");
    luaL_dofile(L, "/usr/local/lib/luaThreading/luaThreading.luac");
    luaL_dofile(L, "/usr/local/lib64/luaThreading/luaThreading.luac");
    luaL_dofile(L, "/usr/lib/luaThreading/luaThreading.luac");
    luaL_dofile(L, "/usr/lib64/luaThreading/luaThreading.luac");
    luaL_dofile(L, "luaThreading.lua");
    lua_getglobal(L, "LUATHREAD_GLOBAL_NAME");
    return 1;
}

