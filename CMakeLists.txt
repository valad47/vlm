cmake_minimum_required(VERSION 3.30)

project(vlm)

include(FetchContent)

option(LUAU_BUILD_CLI "Build CLI" OFF)
option(LUAU_BUILD_TESTS "Build tests" OFF)
option(LUAU_EXTERN_C "Use extern C for all APIs" ON)

set(LUAU_VERSION 0.667)
FetchContent_Declare(
    Luau
    URL https://github.com/luau-lang/luau/archive/refs/tags/${LUAU_VERSION}.tar.gz
)

FetchContent_MakeAvailable(Luau)

# Making possible to use Luau API in C
# https://github.com/luau-lang/luau/pull/933#issuecomment-2152968746
foreach(target Luau.VM Luau.Compiler)
  get_target_property(defs ${target} INTERFACE_COMPILE_DEFINITIONS)
  list(FILTER defs EXCLUDE REGEX [[LUA.*_API]])
  set_target_properties(${target} PROPERTIES INTERFACE_COMPILE_DEFINITIONS "${defs}")
endforeach()

add_executable(vlm src/main.c src/runtime.c src/stdlib.c)
set_property(TARGET vlm PROPERTY CMAKE_C_STANDART C23)

target_link_libraries(vlm Luau.Compiler Luau.VM)
