//Includes
#include "luaThreading.h"
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>

#define REGISTRY_TABLE_NAME  "LUA_THREADING_GLOBAL_CONTEXT"
#define FUNCTION_TABLE_FIELD "__function_table"
#define MUTEX_FIELD          "__mutex"
#define INDEX_FIELD          "index"

#define UD_CTX_INDEX   1
#define UD_FNC_INDEX   2
#define UD_STATE_INDEX 3

//Types definitions
struct lt_thread_ctx {
    lua_State* state;
    pthread_t* thread;
    int number_of_arguments; // Number of argument that will be given to the function
    int function_id; // Id of the function to call in the global list
};

//Static functions definitions
static int lt_runFunc(lua_State* L);
static void* lt_threaded(void* args);
static int lt_closeThread(lua_State* L);
static void lt_swapElem(lua_State* from, int index, lua_State* to);

//Function code

/*
 * This function runs the function given as argument in a new thread and return
 * a thread handle. The thread handle if a userdata with the following elements:
 *  - A lt_thread_ctx struct in a lightuserdata
 *  - The function to run, placed here to prevent garbage collection
 *  - The lua_State of the thread, to prevent garbage collection
 */
static int lt_runFunc(lua_State* L){
    int num_args = lua_gettop(L);
    lua_newuserdatauv(L, 0, 3);
    lua_State* copy = lua_newthread(L);
    lua_setiuservalue(L, -2, UD_STATE_INDEX);
    struct lt_thread_ctx* lt_thread =  malloc(sizeof(struct lt_thread_ctx));
    lt_thread->state = copy; 
    lt_thread->number_of_arguments = num_args-1;

    // Preparing the return value. This value is a table that contains the
    // Puting function in return value
    lua_pushvalue(L, 1);
    lua_setiuservalue(L, -2, UD_FNC_INDEX);

    // Puting function in the global function table
    lua_getfield(L, LUA_REGISTRYINDEX, REGISTRY_TABLE_NAME);
    lua_getfield(L, -1, MUTEX_FIELD);
    pthread_mutex_t* mutex = lua_touserdata(L, -1);
    lua_pop(L, 1);
    pthread_mutex_lock(mutex);
    lua_getfield(L, -1, FUNCTION_TABLE_FIELD);
    lua_getfield(L, -1, INDEX_FIELD);
    int used_index = lua_tointeger(L, -1);
    lua_pushvalue(L, 1);
    lua_settable(L, -3);
    lua_pushinteger(L, used_index+1);
    lua_setfield(L, -2, INDEX_FIELD);
    lua_pop(L, 2);
    pthread_mutex_unlock(mutex);
    lt_thread->function_id = used_index;

    // Reading the function from the global table and pushing it on the stack
    // of the thread
    pthread_mutex_lock(mutex);
    lua_getfield(copy, LUA_REGISTRYINDEX, REGISTRY_TABLE_NAME);
    lua_getfield(copy, -1, FUNCTION_TABLE_FIELD);
    lua_geti(copy, -1, lt_thread->function_id);
    pthread_mutex_unlock(mutex);

    // Creating arguments
    for (int i=0; i<num_args-1; i++) {
        lt_swapElem(L, 2+i, copy);
    }

    // Creating thread
    pthread_t* thread = malloc(sizeof(pthread_t));
	pthread_create(thread, NULL, lt_threaded, (void *) lt_thread);

    // Puting the thread in the context
    lt_thread->thread = thread;
    lua_pushlightuserdata(L, lt_thread);
    lua_setiuservalue(L, -2, UD_CTX_INDEX);
    return 1;
}

/*
 * Run a function from a lua_State in a new thread
 *  Argument:
 *      args : a pointer to a lt_thread_ctx struct, contain a
 *             lua_State pointer, the name of the function to run
 *             and will be added a pointer to the thread where the
 *             lua_State is running
 *  return:
 *      NULL
 */
static void* lt_threaded(void* args){
    struct lt_thread_ctx* lt_args = args;
    lua_call(lt_args->state, lt_args->number_of_arguments, 1);

    // Remove function from the global list
    lua_getfield(lt_args->state, LUA_REGISTRYINDEX, REGISTRY_TABLE_NAME);
    lua_getfield(lt_args->state, -1, MUTEX_FIELD);
    pthread_mutex_t* mutex = lua_touserdata(lt_args->state, -1);
    lua_pop(lt_args->state, 1);
    pthread_mutex_lock(mutex);
    lua_getfield(lt_args->state, -1, FUNCTION_TABLE_FIELD);
    lua_pushinteger(lt_args->state, lt_args->function_id);
    lua_pushnil(lt_args->state);
    lua_settable(lt_args->state, -3);
    lua_pop(lt_args->state, 2);
    pthread_mutex_unlock(mutex);

    return NULL;
}    

/*
 * Used to create the joinThread lua function. Take a thread as an
 * argument, join it and close the lua_State it ran on.
 * Return the result of the function callded when lauching the thread.
 */
static int lt_closeThread(lua_State* L){
    lua_getiuservalue(L, -1, UD_CTX_INDEX);
    struct lt_thread_ctx* lt_thread = lua_touserdata(L, -1);
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

static int global_gc(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, REGISTRY_TABLE_NAME);
    lua_getfield(L, -1, MUTEX_FIELD);
    pthread_mutex_t* mutex = lua_touserdata(L, -1);
    pthread_mutex_destroy(mutex);
    free(mutex);
    lua_pop(L, 2);
    return 0;
}

static void manage_global_context(lua_State* L) {
    if (lua_getfield(L, LUA_REGISTRYINDEX, REGISTRY_TABLE_NAME) == LUA_TTABLE) {
        // Context already generated, no need to do it again
        lua_pop(L, 1);
        return;
    }
    lua_pop(L, 1);
    lua_createtable(L, 0, 2);
    // Makes a table that is used to store functions to call in thread or
    // threads that should be protected from the garbage collector.
    // This table got a field "index" that keeps track or where the next element
    // should be store and a list of elements that are replaced with "nil" when
    // their uses are finished to ensure that they will be garbaged collected.
    lua_createtable(L, 0, 10);
    lua_pushinteger(L, 1);
    lua_setfield(L, -2, INDEX_FIELD);
    lua_setfield(L, -2, FUNCTION_TABLE_FIELD);
    // As that table might be used in multiple threads, we want to protect
    // it with a mutex.
    pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex, NULL);
    lua_pushlightuserdata(L, mutex);
    lua_setfield(L, -2, MUTEX_FIELD);
    // Metatable to free used memory
    lua_createtable(L, 0, 1);
    lua_pushcfunction(L, global_gc);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);
    lua_setfield(L, LUA_REGISTRYINDEX, REGISTRY_TABLE_NAME);
}

int luaopen_luaThreading(lua_State* L) {
    luaL_newlib(L, luaThreading);
    manage_global_context(L);
    return 1;
}

