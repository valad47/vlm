#include "lua.h"
#include "luacode.h"
#include "lualib.h"
#include <stdio.h>
#include <stdlib.h>

lua_State *loadstring(lua_State* L, char *str, size_t size, char *chunkname) {
    size_t bytecode_size;
    char *bytecode = luau_compile(str, size, NULL, &bytecode_size);

    lua_State* ML = lua_newthread(L);
    int result = luau_load(ML, chunkname ? chunkname : "vlm", bytecode, bytecode_size, 0);
    luaL_sandboxthread(ML);

    if(result != LUA_OK) {
        printf("Failed to load bytecode:\n%s\n", lua_tostring(L, -1));
        return NULL;
    }

    free(bytecode);
    return ML;
}
