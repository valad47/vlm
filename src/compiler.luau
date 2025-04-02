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
    luaucompile(v, "build/"..(string.split(v, ".luau")[1])..".vlb")
end

fwrite(
    "build/pkginfo.luau",
    `return \{\n`..
    `    pkgname = "{pkgname}",\n`..
    `}`
)

execute(
    "tar",
    "-cf",
    `{pkgname}`,
    "build"
)
execute(
    "gzip",
    "-fS",
    ".vlp",
    `{pkgname}`
)

print("Succesfully builded package")
