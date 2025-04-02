mkdir(".vlm-tmp")
local pkgname
local _, err = pcall(function()
    execute(
        "tar",
        "--directory=.vlm-tmp",
        "-xf",
        filename
    )
    local pkginfo = require(`.vlm-tmp/build/pkginfo`)
    pkgname = pkginfo.pkgname
    local pkgdir = `{home}/.vlm/{pkgname}`

    --Deleting package folder in case of reinstall
    execute(
        "rm",
        "-rf",
        pkgdir
    )
    execute(
        "mkdir",
        "-p",
        pkgdir
    )
    execute(
        "cp",
        "-rT",
        ".vlm-tmp/build",
        pkgdir
    )
end)

execute("rm", "-rf", ".vlm-tmp")
if err then
   error(err)
end

print(`Succesfully installed package [{pkgname}]`)