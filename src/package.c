#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32

#include <windows.h>
#include <direct.h>

#else

#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#endif


#include "vlmstdlib.h"
#include "runtime.h"
#include "lualib.h"
#include "luacode.h"

int vlm_write(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    const char *input = luaL_checkstring(L, 2);

    FILE *fd = fopen(filename, "w+");
    if(!fd)
        luaL_error(L, "Failed to write file: %s\n", strerror(errno));

    fwrite(input, 1, strlen(input), fd);
    fclose(fd);
    return 0;
}

int vlm_execute(lua_State *L) {
    int argc = lua_gettop(L);

    char **argv = malloc(sizeof(char**) * (argc + 1));
    memset(argv, 0, sizeof(char**) * (argc + 1));
    for(int i = 1; i <= argc; i++) {
        argv[i-1] = luaL_checkstring(L, i);
    }
#ifdef _WIN32
	STARTUPINFO siStartInfo = {};
	PROCESS_INFORMATION piProcInfo = {};
	BOOL bSuccess = CreateProcessA(NULL, *argv, NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo);

	if(!bSuccess) {
		luaL_error(L, "Could not execute command: %s", strerror(errno));
	}
	CloseHandle(piProcInfo.hThread);
	DWORD result = WaitForSingleObject(piProcInfo.hProcess, INFINITE);

	if(result == WAIT_FAILED) {
		luaL_error(L, "Failed on waiting of proccess: %s\n", strerror(errno));
	}

	DWORD status;
	GetExitCodeProcess(piProcInfo.hProcess, &status);
	if(status != 0) {
	    luaL_error(L, "Error during comand execution, exit status: %ld", status);
	}
	CloseHandle(piProcInfo.hProcess);
#else
    pid_t pid = fork();

    if(pid == 0) {
        if(execvp(argv[0], argv) < 0)
            luaL_error(L, "Could not execute command: %s\n", strerror(errno));
     }

    int wstatus = 0;
    if(waitpid(pid, &wstatus, 0) < 0) {
        luaL_error(L, "Failed on waiting of proccess: %s\n", strerror(errno));
    }
    if(WIFEXITED(wstatus)) {
        int status = WEXITSTATUS(wstatus);
        if(status != 0) {
            luaL_error(L, "Error during comand execution, exit status: %d", status);
        }
    }
#endif
    free(argv);
    return 0;
}

int vlm_mkdir(lua_State *L) {
    const char *dir = luaL_checkstring(L, 1);

#ifdef _WIN32
	_mkdir(dir);
#else
	mkdir(dir, 00740);
#endif

    return 0;
}

int vlm_pwd(lua_State *L) {
    lua_pushstring(L, getenv("PWD"));
    return 1;
}

int vlm_cd(lua_State *L) {
#ifdef _WIN32
    _chdir(luaL_checkstring(L, 1));
#else
    chdir(luaL_checkstring(L, 1));
#endif
    return 0;
}

int vlm_getenv(lua_State *L) {
	char *variable = getenv(luaL_checkstring(L, 1));
	if(!variable) return 0;
	lua_pushstring(L, variable);
	return 1;
}

int vlm_luaucompile(lua_State *L) {
    const char *input = luaL_checkstring(L, 1);
    const char *output = luaL_checkstring(L, 2);

    FILE *fd = fopen(input, "r");
    if(!fd) {
        luaL_error(L, "No such file or directory: %s", input);
    }

    fseek(fd, 0, SEEK_END);
    int fsize = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    char *buffer = malloc(fsize+16);
    memset(buffer, 0, fsize+16);
    fread(buffer, fsize+16, 1, fd);
    fclose(fd);

    lua_CompileOptions opt = {
        .optimizationLevel = 2,
        .debugLevel = 1
    };

    size_t bytecode_size = 0;
    char *bytecode = luau_compile(buffer, strlen(buffer), &opt, &bytecode_size);

    fd = fopen(output, "w+");
    if(!fd) {
        luaL_error(L, "Failed to write %s: %d", output, errno);
    }
    fwrite(bytecode, 1, bytecode_size, fd);
    fclose(fd);

    free(buffer);
    free(bytecode);
    return 0;
}

void builder(int argc, char **argv) {
    const char builder[] = {
        #embed "compiler.luau"
        ,'\0'
    };

    lua_State *L = luaL_newstate();

    luaL_Reg reg[] = {
        {"execute", vlm_execute},
        {"luaucompile", vlm_luaucompile},
        {"mkdir", vlm_mkdir},
        {"fwrite", vlm_write},
        {"pwd", vlm_pwd},
        {"cd", vlm_cd},
        {"getenv", vlm_getenv},

        {NULL, NULL}
    };
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, NULL, reg);
    lua_pop(L, 1);
    luaL_openlibs(L);
    vlm_stdinit(L);

    lua_State *ML = loadstring(L, builder, strlen(builder), "vlm_compiler");

    if(lua_resume(ML, L, 0) != LUA_OK) {
        fprintf(stderr, "Builder failure:\n\t%s", lua_tostring(ML, -1));
        exit(1);
    }
    lua_close(L);
    exit(0);
}

void installer(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "No file provided\n");
        exit(1);
    }

    const char installer[] = {
        #embed "installer.luau"
        ,'\0'
    };

    lua_State *L = luaL_newstate();

    luaL_Reg reg[] = {
        {"execute", vlm_execute},
        {"mkdir", vlm_mkdir},
        {"fwrite", vlm_write},
        {"pwd", vlm_pwd},
        {"cd", vlm_cd},
        {"getenv", vlm_getenv},

        {NULL, NULL}
    };
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, NULL, reg);
    lua_pop(L, 1);
    luaL_openlibs(L);
    vlm_stdinit(L);

    lua_pushstring(L, argv[0]);
    lua_setglobal(L, "vlm");

    lua_pushstring(L, argv[2]);
    lua_setglobal(L, "filename");

    lua_pushstring(L, getenv("HOME"));
    lua_setglobal(L, "home");

    lua_State *ML = loadstring(L, installer, strlen(installer), "vlm_installer");

    if(lua_resume(ML, L, 0) != LUA_OK) {
        fprintf(stderr, "Installer failure:\n\t%s", lua_tostring(ML, -1));
        exit(1);
    }
    lua_close(L);
    exit(0);
}
