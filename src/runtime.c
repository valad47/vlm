#include "lua.h"
#include "luacode.h"
#include "lualib.h"
#include <stdio.h>
#include <stdlib.h>

lua_State *loadstring(lua_State* L, const char *str, size_t size, const char *chunkname) {
    size_t bytecode_size;
    char *bytecode = luau_compile(str, size, NULL, &bytecode_size);

    lua_State* ML = lua_newthread(L);
    int result = luau_load(ML, chunkname ? chunkname : "vlm", bytecode, bytecode_size, 0);

    if(result != LUA_OK) {
        fprintf(stderr, "Failed to load bytecode:\n%s\n", lua_tostring(ML, -1));
        exit(1);
    }

    luaL_sandboxthread(ML);

    free(bytecode);
    return ML;
}
