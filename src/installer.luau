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

    if pkginfo.dependency then
        local exists, pkgremotes = pcall(require, "/etc/vlm-remotes")
        if exists then
            for i,v in pkginfo.dependency do
                local exists = pcall(execute, "test", "-d", `{home}/.vlm/{v}`)
                if exists then continue end

                if not pkgremotes[v] then
                    print(`[WARNING] you don't have module [{v}] installed, which is required by module [{pkgname}]`)
                    continue
                end
                execute("git", "clone", "-q", "--recursive", "--depth", "1", pkgremotes[v], `.vlm-tmp/{v}`)
                cd(`.vlm-tmp/{v}`)
                execute("vlm", "build")
                local deppkg = require("build/pkginfo")
                execute("vlm", "install", `{deppkg.pkgname}-{deppkg.pkgver}.vlp`)
                cd("../..")
            end
        end
   end
end)

execute("rm", "-rf", ".vlm-tmp")
if err then
    error(err)
end

print(`Succesfully installed package [{pkgname}]`)
