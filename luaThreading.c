#include "luaThreading.h"

/*
 * Used to create the launchThread lua function which take the name of
 * a function as argument a return a thread object
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
    lua_call(lt_args->state, 0, 1);
    return NULL;
}    

/*
 * Used to create the joinThread lua function. Take a thread as an
 * argument, join it and close the lua_State it ran on.
 * Return the result of the function callded when lauching the thread.
 */
int lt_closeThread(lua_State* L){
    struct lt_threaded_thread* lt_thread = (struct lt_threaded_thread*) lua_tointeger(L, 1);
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
void lt_swapElem(lua_State* from, lua_State* to){
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
        default:
            fprintf(stderr, "Error: unknow or invalid type on top of a stack.\n");
    }
}

void lt_include(lua_State* L){
    luaL_dofile(L, "luaThreading.lua");
    lua_pushcfunction(L, lt_runFunc);
    lua_setglobal(L, "_launchThread");
    lua_pushcfunction(L, lt_closeThread);
    lua_setglobal(L, "joinThread");
}

