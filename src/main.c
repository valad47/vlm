#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "runtime.h"
#include "lua.h"
#include "lualib.h"
#include "vlmstdlib.h"
#include "package.h"

void vlm_run(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "No file provided");
        exit(1);
    }

    FILE *fd = fopen(argv[2], "r");
    if(!fd){
        fprintf(stderr, "Cannot open file [%s]\n", argv[2]);
        exit(1);
    }
    fseek(fd, 0, SEEK_END);
    int fsize = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    char *buffer = malloc(fsize+16);
    memset(buffer, 0, fsize+16);
    fread(buffer, fsize+16, 1, fd);

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    vlm_stdinit(L);

    lua_State *ML = loadstring(L, buffer, strlen(buffer), argv[2]);

    free(buffer);

    if(lua_resume(ML, L, 0) != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(ML, -1));
        exit(1);
    }
    lua_close(L);
}

const char help_message[] =
    "Usage: %s <command> <file>\n"
    "\n"
    "Commands:\n"
    "\trun\t- run script\n"
    "\thelp\t- show this message\n"
    "\tbuild\t- build a .vlp package\n"
    "\tinstall\t- install a .vlp package\n"
;

int main(int argc, char **argv) {
    if(argc <= 1) {
        printf(help_message, argv[0]);
        exit(0);
    }

    if(!strcmp("help", argv[1])) {
        printf(help_message, argv[0]);
        return 0;
    }
    else if(!strcmp("run", argv[1])) {
        vlm_run(argc, argv);
        return 0;
    }
    else if(!strcmp("build", argv[1])) {
        builder(argc, argv);
        return 0;
    }
    else if(!strcmp("install", argv[1])) {
        installer(argc, argv);
        return 0;
    }

    printf("Invalid command [%s]\n", argv[1]);
    return 1;
}
