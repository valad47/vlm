#include "runtime.h"
#include "lua.h"
#include "lualib.h"
#include "luacode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

#include <windows.h>
#include <direct.h>

#else

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>

#endif

void load_so(lua_State *L, int idx, const char* module_name) {
    char path[1024] = {0};
    sprintf(path, 
#ifdef _WIN32
	"%s\\%s\\%s\\%s.dll",
#else
    "%s/%s/%s/%s.so",
#endif
                  getenv("HOME"),
                  VLM_DIR,
                  module_name,
                  module_name
    );

    void *handle = 
#ifdef _WIN32
	LoadLibrary(path);
#else
    dlopen(path, RTLD_LAZY);
#endif
    if(!handle){
        luaL_error(L, "Module failed to load %s.so: %s\n", module_name, "PENIS");
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
        lua_CFunction mfunc = 
	#ifdef _WIN32
		(lua_CFunction)GetProcAddress(handle, fname);
	#else
        dlsym(handle, fname);
	#endif
        if(!mfunc) {lua_pop(L, 1); continue;}
        lua_pushcfunction(L, mfunc, fname);
        lua_setfield(L, idx, lname);
        lua_pop(L, 1);
    }
}

char *strshrink(const char *str, char target) {
    size_t size = strlen(str);
    char *copy = malloc(size+1);
    strcpy(copy, str);

    int last_char = 0;

    for(char *s = copy; *s != 0; s++) {
        if(*s == target)
            last_char = s - copy;
    }

    if (last_char == 0) {
        free(copy);
        return NULL;
    }
    copy[last_char+1] = 0;
    return copy;
}

int vlm_require(lua_State *L) {
    const char *module_name = luaL_checkstring(L, 1);
    char module_file[512] = {0};
    FILE *fd = 0;
    int vlb = 0;

    struct lua_Debug dinfo = {0};
    lua_getinfo(L, 1, "s", &dinfo);

    //First trying to search for a file relative to current file position
    char *spath = strshrink(dinfo.source, '/');
    sprintf(module_file, "%s%s.luau", spath == NULL ? "":spath, module_name);


    //Checking if it's not the same file we're running
    if(strcmp(dinfo.source, module_file))
        fd = fopen(module_file, "r");

    if(!fd) {
        sprintf(module_file, "%s%s.vlb", spath == NULL ? "":spath, module_name);
        vlb = 1;
        fd = fopen(module_file, "r");
    }
    free(spath);
    //If file was not found, search in vlm modules folder
    if(!fd) {
        vlb = 0;
        char *home = getenv(
		#ifdef _WIN32
			"APPDATA"		
		#else
        	"HOME"
        #endif
        );
        char vlm_dir[512] = {0};
        sprintf(vlm_dir, "%s/%s", home, VLM_DIR);

#ifdef _WIN32
		if(GetFileAttributesA(vlm_dir) == INVALID_FILE_ATTRIBUTES) {
			_mkdir(vlm_dir);
            luaL_error(L, "Module not found: [%s]", module_name);
		}
#else
        struct stat st = {0};
        if(stat(vlm_dir, &st) == -1) {
            mkdir(vlm_dir, 00740);
            luaL_error(L, "Module not found: [%s]", module_name);
        }
#endif

        //First we check for .vlb luau bytecode
        sprintf(module_file, "%s/%s/%s.vlb",	vlm_dir, module_name, module_name);
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

    if(luau_load(ML, module_file, bytecode, bytecode_size, 0) == 0) {
        int status = lua_resume(ML, NULL, 0);

        if (status == 0) {
            if (lua_gettop(ML) == 0)
                luaL_error(L, "module must return a value");
            else if (!lua_istable(ML, -1) && !lua_isfunction(ML, -1))
                luaL_error(L, "module must return a table or function");
       } else {
           luaL_error(L, "[%d]Module contains an error:\n\t%s\n", status, lua_tostring(ML, -1));
       }
    } else {
        luaL_error(L, "Module contains an error:\n\t%s\n", lua_tostring(ML, -1));
    }

    if(lua_istable(ML, 1)) {
    lua_getfield(ML, 1, "LOAD_SO");
    if(lua_toboolean(ML, -1)) {
        lua_pop(ML, 1);
        lua_pushnil(ML);
        lua_setfield(ML, 1, "LOAD_SO");

        load_so(ML, 1, module_name);
    } else
        lua_pop(ML, 1);
    }

    lua_xmove(ML, L, 1);

    fclose(fd);
    free(buffer);
    if(!vlb) free(bytecode);

    return 1;
}

int vlm_require_shared(lua_State *L) {
    lua_State* GL = lua_mainthread(L);
    const char *module_name = luaL_checkstring(L, 1);

    lua_getglobal(GL, "__MODULES");
    if(!lua_istable(GL, -1)) {
        lua_pop(GL, 1);
        lua_createtable(GL, 0, 1);
        lua_pushvalue(GL, -1);
        lua_setglobal(GL, "__MODULES");
    }

    lua_getfield(GL, -1, luaL_checkstring(L, 1));
    if(!lua_istable(GL, -1) && !lua_isfunction(GL, -1)) {
        lua_pop(GL, 1);
        vlm_require(L);
        lua_pushvalue(L, -1);
        lua_xmove(L, GL, 1);
        lua_setfield(GL, -2, module_name);
        lua_pop(GL, 1);
        return 1;
    }
    lua_xmove(GL, L, 1);
    lua_pop(GL, 1);

    return 1;
}

int vlm_stdinit(lua_State *L) {
    static const luaL_Reg reg[] = {
        {"require", vlm_require},
        {"require_shared", vlm_require_shared},
        {NULL, NULL}
    };
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, NULL, reg);
    lua_pop(L, 1);

    lua_newtable(L);
    lua_setglobal(L, "__MODULES");

#ifdef _WIN32
	lua_pushboolean(L, true);
	lua_setglobal(L, "_WIN32");
#endif

	lua_pushstring(L, VLM_VERSION);
	lua_setglobal(L, "_VLM");

    return 0;
}
