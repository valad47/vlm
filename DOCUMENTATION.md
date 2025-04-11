# Creating package

## Writing package

Every package must have at least 1 `.luau` file, that containg function definitions in a table, and then return it, for example:

```luau
local foo = {}

function foo.foo()
    print("foo")
end

function foo.bar(str: string) end

return foo
```

We have function `foo.bar()` body empty, but why? We can owerride this function using C language! To do so, we have to set `LOAD_SO` to `true`, like this:



```luau
--[[...]]

foo.LOAD_SO = true

--[[...]]
```

If this enabled, `vlm` will try to search for `.so` shared library, that we wrote using C.

## C binding

C functions, that will owerride luau functions, must have exact function name, or additionaly have prefix, that we specify like so:

`foo.luau`
```luau
--[[...]]

foo.SO_PREFIX = "foo_"

--[[...]]
```

And then we can write C file:

`foo.c`
```c
#include <stdio.h>
#include "lua.h"
#include "lualib.h"

int foo_bar(lua_State *L) {
    printf("foo %s bar", luaL_checkstring(L, 1));

    return 1;
}
```

Notice, that we have to use [Lua API](https://www.lua.org/manual/5.1/manual.html) to bind functions. With this power, we can bind all C libraries we want and even more!

## Building
To build package, you have to make `build.luau` file, with info about package. `build.luau` must return a table with package info, for example:

```luau
return {
    pkgname = "foo",
    pkgver = "v4.2",
    luau_files = {
        "foo.luau"
    },
    c_files = {
        "foo.c"
    },
    c_args = {
        "-lm"
    },
    dependency = {
        "bar"
    }
}
```

- `pkgname` - name of the package
- `pkgver` - _optional_ package version
- `luau_files` - array of luau files, that will be compiled into `.vlb` bytecode _Note: one of luau files must have to be named exactly like package name, for example `foo.luau`_
- `c_files` - _optional_ C files, that will be compiled into `.so` shared library
- `c_args` - _optional_ arguments that will be passed into C compiler
- `dependency` - _optional_ packages, on which module depends. If these packages is contained in remotes, then they will be automatically installed during package installetion

And then, use command
```sh
vlm build
```
to build package, and get `foo-v4.2.vlp` package

# Installing package

To install package, we have to use command
```sh
vlm install foo-v4.2.vlp
```
Where `foo-v4.2.vlp` is a package we built

# Using packages

To use packages in luau, we simply `require()` them like so:

```luau
local foo = require("foo")

foo.bar("bar")
```

And then run it using `vlm`:
```sh
vlm run foo.luau
```

And that's all! Simple enough
