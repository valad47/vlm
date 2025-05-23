vlm
====

vlm (valad Luau Manager) is a package manager and runtime for [Luau](https://luau.org) language.

At this point it works only for Linux

# Building

To build executable, use this command:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang
cmake --build build -j4 -t vlm vlmruntime
```

# Documentation

Read vlm ducumentation in [DOCUMENTATION](https://github.com/valad47/vlm/blob/main/DOCUMENTATION.md)

# Conception

All packages/modules is located in `$HOME/.vlm`, and every module is in separate folder.
Everything vlm does, is searching for module in this folder, for example `$HOME/.vlm/foo/foo.luau` and loads it. If module's return table have value `LOAD_SO` set to `true`, then vlm searches for `.so` file in same folder, e.g. `$HOME/.vlm/foo/foo.so`, and replaces functions in module's table, if it is contained in dynamic library.
That makes it really easy to bind any C library into Luau language, or combine C and Luau to make vlm library.
