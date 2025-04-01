#include "runtime.h"
#include "lua.h"
#include "lualib.h"
#include "luacode.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <dlfcn.h>

void load_so(lua_State *L, int idx, const char* module_name) {
    char path[1024] = {0};
    sprintf(path, "%s/%s/%s/%s.so",
                  getenv("HOME"),
                  VLM_DIR,
                  module_name,
                  module_name
    );
    void *handle = dlopen(path, RTLD_LAZY);
    if(!handle){
        luaL_error(L, "Module failed to load %s.so: %s\n", module_name, dlerror());
        return;
    }

    const char *prefix = "";

    lua_getfield(L, idx, "SO_PREFIX");
    if(lua_isstring(L, -1)) {
        prefix = lua_tostring(L, -1);
    }
    lua_pop(L, 1);

    lua_pushnil(L);
    while(lua_next(L, idx) != 0) {
        if(!lua_isstring(L, -2) || !lua_isfunction(L, -1)) {lua_pop(L, 1); continue;}
        const char *lname = lua_tostring(L, -2);
        char fname[256] = {0};
        sprintf(fname, "%s%s",
                prefix,
                lname
        );
        lua_CFunction mfunc = dlsym(handle, fname);
        if(!mfunc) {lua_pop(L, 1); continue;}
        lua_pushcfunction(L, mfunc, fname);
        lua_setfield(L, idx, lname);
        lua_pop(L, 1);
    }
}

int vlm_require(lua_State *L) {
    const char *module_name = luaL_checkstring(L, 1);
    char module_file[512] = {0};
    FILE *fd = 0;
    int vlb = 0;

    //First trying to search for a file in local dir
    sprintf(module_file, "%s.luau", module_name);

    //Checking if it's not the same file we're running
    struct lua_Debug dinfo = {0};
    lua_getinfo(L, 1, "s", &dinfo);
    if(strcmp(dinfo.source, module_file))
        fd = fopen(module_file, "r");

    //If file was not found, search in vlm modules folder
    if(!fd) {
        char *home = getenv("HOME");
        char vlm_dir[512] = {0};
        sprintf(vlm_dir, "%s/%s", home, VLM_DIR);

        struct stat st = {0};
        if(stat(vlm_dir, &st) == -1) {
            mkdir(vlm_dir, 00740);
            luaL_error(L, "Module not found: [%s]", module_name);
        }

        //First we check for .vlb luau bytecode
        sprintf(module_file, "%s/%s/%s.vlb", vlm_dir, module_name, module_name);
        fd = fopen(module_file, "r");
        if(fd) {
            vlb = 1;
            goto next;
        }

        sprintf(module_file, "%s/%s/%s.luau", vlm_dir, module_name, module_name);
        fd = fopen(module_file, "r");
        if(!fd) luaL_error(L, "Module not found: [%s]", module_name);
    }

next:
    fseek(fd, 0, SEEK_END);
    int fsize = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    char *buffer = malloc(fsize+16);
    memset(buffer, 0, fsize+16);
    fread(buffer, fsize+16, 1, fd);

    size_t bytecode_size;
    char *bytecode;
    if(vlb) {
        bytecode_size = fsize;
        bytecode = buffer;
    } else {
        bytecode = luau_compile(buffer, strlen(buffer), NULL, &bytecode_size);
    }

    lua_State *GL = lua_mainthread(L);
    lua_State *ML = lua_newthread(GL);
    lua_xmove(GL, L, 1);

    luaL_sandboxthread(ML);

    if(luau_load(ML, module_name, bytecode, bytecode_size, 0) == 0) {
        int status = lua_resume(ML, NULL, 0);

        if (status == 0) {
            if (lua_gettop(ML) == 0)
                luaL_error(L, "module must return a value");
            else if (!lua_istable(ML, -1) && !lua_isfunction(ML, -1))
                luaL_error(L, "module must return a table or function");
        }
    }

    lua_getfield(ML, 1, "LOAD_SO");
    if(lua_toboolean(ML, -1)) {
        lua_pop(ML, 1);
        lua_pushnil(ML);
        lua_setfield(ML, 1, "LOAD_SO");

        load_so(ML, 1, module_name);
    } else
        lua_pop(ML, 1);

    lua_xmove(ML, L, 1);

    fclose(fd);
    free(buffer);
    if(!vlb) free(bytecode);

    return 1;
}

int vlm_stdinit(lua_State *L) {
    static const luaL_Reg reg[] = {
        {"require", vlm_require},
        {NULL, NULL}
    };
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, NULL, reg);
    lua_pop(L, 1);

    return 0;
}
