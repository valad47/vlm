local build_file
local _, err = pcall(function()
    build_file = require("build")
end)
if err then
   error(`Currend directory does not contains [build.luau], or error:\n{err}`)
end

local pkgname = build_file.pkgname
local compiler = build_file.compiler or getenv("C_COMPILER") or "cc"

mkdir("build")
if build_file.c_files then
   local executable = "build/"..pkgname.. if _WIN32 then ".dll" else ".so";
   if build_file.c_args then
       table.move(build_file.c_args, 1, #build_file.c_args, #build_file.c_files + 1, build_file.c_files)
   end
   execute(
       compiler,
       "-shared",
       "-o",
       executable,
       if _WIN32 then
		   `-I{home}/.vlm/include/vlm`
       else
           "-I/usr/include/vlm",
       "-ggdb",
       "-lvlmruntime",
       "-O2",
       "-fPIC",
       table.unpack(build_file.c_files)
   )
end
for i, v in build_file.luau_files do
    local out = string.split(v, "/")
    out = string.split(out[#out], ".lua")
    out = out[1]
    luaucompile(v, "build/"..
        out..
        ".vlb")
end

local pkginfo =
    `return \{\n`..
    `    pkgname = "{pkgname}",\n`..
    `    pkgver = "{build_file.pkgver}",\n`..
    `    dependency = \{\n`

if build_file.dependency then
    for i,v in build_file.dependency do
       pkginfo = pkginfo..`        "{v}",\n`
    end
end

pkginfo =
    pkginfo..
    "    }\n"..
    "}"

fwrite("build/pkginfo.luau", pkginfo)

execute(
    "tar",
    "-czf",
    `{pkgname}-{build_file.pkgver}.vlp`,
    "build"
)

print(`Succesfully builded package [{pkgname}]`)
