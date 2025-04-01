#pragma once

#include "lua.h"
#define VLM_DIR ".vlm"

lua_State *loadstring(lua_State* L, const char *str, size_t size, const char *chunkname);
