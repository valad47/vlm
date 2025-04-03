local build_file
local _, err = pcall(function()
    build_file = require("build")
end)
if err then
   error("Currend directory does not contains [build.luau]")
end

local pkgname = build_file.pkgname
local compiler = build_file.compiler or "clang"

mkdir("build")
if build_file.c_files then
   local executable = "build/"..pkgname..".so";
   execute(
       compiler,
       "-shared",
       "-o",
       executable,
       "-I/usr/include/vlm",
       "-lvlmruntime",
       "-O2",
       (if build_file.c_args then table.unpack(build_file.c_args) else ""),
       table.unpack(build_file.c_files)
   )
   execute("strip", executable)
end
for i, v in build_file.luau_files do
    local out = string.split(v, "/")
    out = string.split(out[#out], ".lua")
    out = out[1]
    luaucompile(v, "build/"..
        out..
        ".vlb")
end

fwrite(
    "build/pkginfo.luau",
    `return \{\n`..
    `    pkgname = "{pkgname}",\n`..
    `    pkgver = "{build_file.pkgver}"\n`..
    `}`
)

execute(
    "tar",
    "-cf",
    `{pkgname}-{build_file.pkgver}`,
    "build"
)
execute(
    "gzip",
    "-fS",
    ".vlp",
    `{pkgname}-{build_file.pkgver}`
)

print("Succesfully builded package")
