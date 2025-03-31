#include <stdio.h>

#include "runtime.h"
#include "lua.h"
#include "lualib.h"
#include "vlmstdlib.h"

int main(int argc, char** argv) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    vlm_stdinit(L);

    char str[] = "local penis = require(\"penis\")\npenis.print(\"vlm 0.0\\n\")\npenis.penis()";
    lua_State *ML = loadstring(L, str, sizeof(str));

    if(lua_resume(ML, L, 0) != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(ML, -1));
        return 1;
    }
    lua_close(L);
}
