#include "luaStateGen.h"

stateGenerator* lsg_init(int numberOfFiles, const char** files, int numberOfFunc, const void* functions){
    stateGenerator* ret = malloc(sizeof(stateGenerator_struct));
    ret->numberOfFiles = numberOfFiles;
    ret->files = files;
    ret->numberOfFunc = numberOfFunc;
    ret->functions = functions;
    return ret;
}

lua_State* lsg_make_state(stateGenerator* sg){
    lua_State* ret;
    ret = luaL_newstate();
    luaL_openlibs(ret);
    for(int i=0; i<sg->numberOfFiles; i++)
        luaL_dofile(ret, sg->files[i]);
    for(int i=0; i<sg->numberOfFunc; i++){
        void (*fnc)(lua_State*);
        fnc = sg->functions[i];
        fnc(ret);
    }
    return ret;
}

