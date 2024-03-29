#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>

/* -------------------------- Internal table fields ------------------------- */

/*
 * The various table used to represent state in the library are precedes by two
 * underscore to highlight the fact that they should not be used by library
 * users.
 */

#define REGISTRY_TABLE_NAME  "LUA_THREADING_GLOBAL_CONTEXT"
#define FUNCTION_TABLE_FIELD "__global_table"
#define MUTEX_FIELD          "__mutex"
#define INDEX_FIELD          "__index"

#define CTX_FIELD      "__ctx"
#define FUNCTION_FIELD "__function"
#define STATE_FIELD    "__state"

/* ------------------------------- Global list ------------------------------ */

/*
 * Lua Threading manages a global list stored in the Lua registry. This list is
 * used to protect some element from garbage collecting and to safely exchange
 * data between threads. This list is protected with a mutex, making it safe
 * but a bit heavy to use.
 */


/*
 * This function prepares everything to access the global list of Lua Threading.
 * It gets the global mutex and takes it, put the global list on top of the
 * stack and return the mutex.
 */
static pthread_mutex_t* prepare_global_list_access(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, REGISTRY_TABLE_NAME);
    lua_getfield(L, -1, MUTEX_FIELD);
    pthread_mutex_t* mutex = lua_touserdata(L, -1);
    lua_pop(L, 1);
    pthread_mutex_lock(mutex);
    lua_getfield(L, -1, FUNCTION_TABLE_FIELD);
    int index = lua_gettop(L);
    lua_rotate(L, index-1, 1);
    lua_pop(L, 1);
    return mutex;
}

/*
 * This function puts the element on top of the Lua stack in the global list
 * at the relevent index and update the index.
 * Return the used index.
 */
static int add_to_global_list(lua_State* L) {
    int top_of_the_stack = lua_gettop(L);
    pthread_mutex_t* mutex = prepare_global_list_access(L);
    lua_getfield(L, -1, INDEX_FIELD);
    int used_index = lua_tointeger(L, -1);
    lua_pushvalue(L, top_of_the_stack);
    lua_settable(L, -3);
    lua_pushinteger(L, used_index+1);
    lua_setfield(L, -2, INDEX_FIELD);
    lua_pop(L, 1);
    pthread_mutex_unlock(mutex);
    return used_index;
}

/*
 * This function puts on top of the stack the element from the global list at
 * the given index.
 */
static void get_from_global_list(lua_State* L, int index) {

    pthread_mutex_t* mutex = prepare_global_list_access(L);
    lua_pushinteger(L, index);
    lua_gettable(L, -2);
    pthread_mutex_unlock(mutex);
    int top_of_the_stack = lua_gettop(L);
    lua_rotate(L, top_of_the_stack-1, 1);
    lua_pop(L, 1);
}

/*
 * This function replace the value at the given index of the global list
 * with a nil.
 */
static void clear_from_global(lua_State* L, int index) {
    pthread_mutex_t* mutex = prepare_global_list_access(L);
    lua_pushinteger(L, index);
    lua_pushnil(L);
    lua_settable(L, -3);
    lua_pop(L, 1);
    pthread_mutex_unlock(mutex);
}

/* --------------------------------- Mutexes -------------------------------- */

/*
 * Lua Threading can generates mutexes that are given to the Lua space. They can
 * be locked and unlocked and are automatically garbage collected.
 */

/*
 * Frees the memory used by a mutex.
 */
static int del_mutex(lua_State* L) {
    lua_getfield(L, -1, MUTEX_FIELD);
    pthread_mutex_t* mutex = lua_touserdata(L, -1);
    pthread_mutex_destroy(mutex);
    free(mutex);
    lua_pop(L, 1);
    return 0;
}

/*
 * Two functions to lock and unlock a mutex.
 */
static int lock_mutex(lua_State* L) {
    lua_getfield(L, -1, MUTEX_FIELD);
    pthread_mutex_t* mutex = lua_touserdata(L, -1);
    pthread_mutex_lock(mutex);
    lua_pop(L, 1);
    return 0;
}
static int unlock_mutex(lua_State* L) {
    lua_getfield(L, -1, MUTEX_FIELD);
    pthread_mutex_t* mutex = lua_touserdata(L, -1);
    pthread_mutex_unlock(mutex);
    lua_pop(L, 1);
    return 0;
}

/*
 * Creates a new mutex and give in to the Lua space. The mutex can be taken,
 * released, and is cleaned when garbage collected.
 */
static int lt_new_mutex(lua_State* L) {
    lua_createtable(L, 0, 3);
    // Generate actual mutex
    pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex, NULL);
    lua_pushlightuserdata(L, mutex);
    lua_setfield(L, -2, MUTEX_FIELD);
    // Add garbage collection
    lua_createtable(L, 0, 1);
    lua_pushcfunction(L, del_mutex);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);
    // Add lock and unlock methods
    lua_pushcfunction(L, lock_mutex);
    lua_setfield(L, -2, "lock");
    lua_pushcfunction(L, unlock_mutex);
    lua_setfield(L, -2, "unlock");
    return 1;
}

/* --------------------------------- Threads -------------------------------- */

struct lt_thread_ctx {
    lua_State* state;
    pthread_t* thread;
    int number_of_arguments; // Number of argument that will be given to the function
    int function_id;         // Id of the function to call in the global list
    int ctx_id;              // Id of the Lua thread handle in the global list
};

/*
 * Push an element of a lua_State to the to of an other lua_State
 *  Arguments:
 *      from : The state we take an element from
 *      index : the position of the element in from
 *      to : the state we put the element on
 */
static void swap_elem(lua_State* from, int index, lua_State* to){
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
        case LUA_TLIGHTUSERDATA:
            lua_pushlightuserdata(to, lua_touserdata(from, index));
            break;
        case LUA_TFUNCTION:
            if(lua_iscfunction(from, index)){
                lua_CFunction func = lua_tocfunction(from, index);
                lua_pushcfunction(to, func);
                break;
            } else {
                goto lua_function;
            }
        case LUA_TTHREAD:
        case LUA_TTABLE:
        case LUA_TUSERDATA:
        lua_function:
            lua_pushvalue(from, index);
            int value_index = add_to_global_list(from);
            lua_pop(from, 1);
            get_from_global_list(to, value_index);
            clear_from_global(to, value_index);
            break;
        default:
            fprintf(stderr, "Error: unknow or invalid type on top of a stack.\n");
    }
}

/*
 * Used to create the join thread method. Take a thread as an
 * argument, join it and close the lua_State it ran on.
 * Return the result of the function callded when lauching the thread.
 */
static int join_thread(lua_State* L){
    lua_getfield(L, -1, CTX_FIELD);
    struct lt_thread_ctx* lt_thread = lua_touserdata(L, -1);
    // Closing the thread
    pthread_join(*(lt_thread->thread), NULL);
    // Putting the result value
    swap_elem(lt_thread->state, -1, L);
    // Clear the context from the global table
    clear_from_global(L, lt_thread->ctx_id);
    // Freeing everyone
    lua_resetthread(lt_thread->state); // Warning: Deprecated in latest Lua, see how things go...
    free(lt_thread->thread);
    free(lt_thread);
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
static void* run_in_thread(void* args){
    struct lt_thread_ctx* lt_args = args;
    lua_call(lt_args->state, lt_args->number_of_arguments, 1);

    // Remove function from the global list
    clear_from_global(lt_args->state, lt_args->function_id);

    return NULL;
}    

/*
 * This function runs the function given as argument in a new thread and return
 * a thread handle. The thread handle if a userdata with the following elements:
 *  - A lt_thread_ctx struct in a lightuserdata
 *  - The function to run, placed here to prevent garbage collection
 *  - The lua_State of the thread, to prevent garbage collection
 */
static int lt_launch_thread(lua_State* L){
    int num_args = lua_gettop(L);
    lua_createtable(L, 0, 4);
    lua_State* copy = lua_newthread(L);
    lua_setfield(L, -2, STATE_FIELD);
    struct lt_thread_ctx* lt_thread =  malloc(sizeof(struct lt_thread_ctx));
    lt_thread->state = copy; 
    lt_thread->number_of_arguments = num_args-1;

    // Preparing the return value. This value is a table that contains the
    // Putting function in return value
    lua_pushvalue(L, 1);
    lua_setfield(L, -2, FUNCTION_FIELD);

    // Putting function in the global function table
    lua_pushvalue(L, 1);
    lt_thread->function_id = add_to_global_list(L);
    lua_pop(L, 1);

    // Reading the function from the global table and pushing it on the stack
    // of the thread
    get_from_global_list(copy, lt_thread->function_id);

    // Creating arguments
    for (int i=0; i<num_args-1; i++) {
        swap_elem(L, 2+i, copy);
    }

    // Creating thread
    pthread_t* thread = malloc(sizeof(pthread_t));
	pthread_create(thread, NULL, run_in_thread, (void *) lt_thread);

    // Putting the thread in the context
    lt_thread->thread = thread;
    lua_pushlightuserdata(L, lt_thread);
    lua_setfield(L, -2, CTX_FIELD);

    // Adding the join method to the thread
    lua_pushcfunction(L, join_thread);
    lua_setfield(L, -2, "join");

    // Store the Lua thread handle into the global list to ensure that it will
    // not be garbage collected. This will cause memory leak if you spawn a lot
    // of threads without joining them, but if you do that, you already messed
    // up.
    int handle_id = add_to_global_list(L);
    lt_thread->ctx_id = handle_id;
    return 1;
}
/* --------------------------------- Library -------------------------------- */

/*
 * Importing the library does two things. Not only does it creates the table
 * exposing the functions as does any other library, but it also initializes the
 * global list used by the library.
 */

/*
 * Frees the mutex protecting the global list, only used when the Lua state is
 * being closed.
 */
static int global_gc(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, REGISTRY_TABLE_NAME);
    del_mutex(L);
    return 0;
}

/*
 * Tries to see if the global state is already initialized (first file importing
 * the library). If it is the case, nothing needs to be done. If it is not the
 * case, the list and mutex needs to be created. As we can be sure that no
 * threads are running if the list is not initialized, we can be sure that no
 * race condition can prevent this function from working correctly.
 */
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

static const struct luaL_Reg luathreading [] = {
    {"launch_thread", lt_launch_thread},
    {"new_mutex", lt_new_mutex},
    {NULL, NULL} /* sentinel */
};

int luaopen_luathreading(lua_State* L) {
    luaL_newlib(L, luathreading);
    manage_global_context(L);
    return 1;
}

