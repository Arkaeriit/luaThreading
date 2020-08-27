#include "luaStateGen.h"

/*
 * Create a state generator, used to create a luaState, load files into
 * it and apply functions to it.
 *  Arguments:
 *      numberOfFiles : the number of files to load
 *      files : a list of all the files which need to be loaded
 *      function : a function to apply on the luaState (input NULL if 
 *                 no functions need to be applied)
 *  return:
 *      A pointer to a state generator
 */
stateGenerator* lsg_init(int numberOfFiles, const char** files, include_fnc function){
    stateGenerator* ret = malloc(sizeof(struct stateGenerator_struct));
    ret->numberOfFiles = numberOfFiles;
    ret->files = files;
    ret->function = function;
    return ret;
}

/*
 * Create a luaState from a state generator
 *  Argument:
 *      sg : a pointer to a stateGenerator
 *  return:
 *      A pointer to a luaState with the std lib and any needed files
 *      loaded to it and a functions applied to it
 */
lua_State* lsg_makeState(stateGenerator* sg){
    lua_State* ret;
    ret = luaL_newstate();
    luaL_openlibs(ret);
    for(int i=0; i<sg->numberOfFiles; i++)
        luaL_dofile(ret, sg->files[i]);
    if(sg->function != NULL){
        include_fnc fnc;
        fnc = sg->function;
        fnc(ret);
    }
    lua_pushinteger(ret, (uint64_t) sg);
    lua_setglobal(ret, "STATE_GENERATOR");
    return ret;
}

/*
 * Regenerate a luaState from the stateGenerator used on it
 *  Arguments:
 *      L : a lua_State pointer created with lsg_makeState
 *  return:
 *      A copy of what L was when generated
 */
lua_State* lsg_regenState(lua_State* L){
    lua_getglobal(L, "STATE_GENERATOR");
    uint64_t addr = lua_tointeger(L,-1);
    stateGenerator* sg = (stateGenerator*) addr;
    return lsg_makeState(sg);
}

/*
 * Close a lua_State and free it's stateGenerator
 *  Argument:
 *      L : a lua_State pointer generated with lsg_makeState
 */
void lsg_purge(lua_State* L){
    lua_getglobal(L, "STATE_GENERATOR");
    uint64_t addr = lua_tointeger(L,-1);
    stateGenerator* sg = (stateGenerator*) addr;
    free(sg);
    lua_close(L);
}

