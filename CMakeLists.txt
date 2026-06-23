cmake_minimum_required(VERSION 3.30)

project(vlm)
set(VLM_VERSION "0.9")

include(FetchContent)

option(LUAU_BUILD_CLI "Build CLI" OFF)
option(LUAU_BUILD_TESTS "Build tests" OFF)
option(LUAU_EXTERN_C "Use extern C for all APIs" ON)

set(LUAU_VERSION 0.714)
FetchContent_Declare(
    Luau
    URL https://github.com/luau-lang/luau/archive/refs/tags/${LUAU_VERSION}.tar.gz
)

FetchContent_MakeAvailable(Luau)

# Making possible to use Luau API in C
# https://github.com/luau-lang/luau/pull/933#issuecomment-2152968746
foreach(target Luau.VM Luau.Compiler Luau.CodeGen)
  get_target_property(defs ${target} INTERFACE_COMPILE_DEFINITIONS)
  list(FILTER defs EXCLUDE REGEX [[LUA.*_API]])
  set_target_properties(${target} PROPERTIES INTERFACE_COMPILE_DEFINITIONS "${defs}")
endforeach()

add_library(vlmruntime SHARED src/runtime.c)
target_compile_options(Luau.Compiler PUBLIC -fPIC)
target_compile_options(Luau.CodeGen PUBLIC -fPIC)
target_compile_options(Luau.VM PUBLIC -fPIC)
target_compile_options(Luau.Ast PUBLIC -fPIC)
target_link_libraries(vlmruntime PRIVATE Luau.Compiler Luau.VM Luau.Ast)

add_executable(vlm src/main.c src/stdlib.c src/package.c)
target_include_directories(vlm PRIVATE
    ${luau_SOURCE_DIR}/VM/include
    ${luau_SOURCE_DIR}/Compiler/include
    ${luau_SOURCE_DIR}/CodeGen/include
)
set_property(TARGET vlm PROPERTY C_STANDARD 23)
target_link_libraries(vlm PRIVATE vlmruntime)
target_compile_definitions(vlm PUBLIC VLM_VERSION="${VLM_VERSION}")

install(TARGETS vlm DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(TARGETS vlmruntime DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES vlm-remotes.luau DESTINATION "/etc")
