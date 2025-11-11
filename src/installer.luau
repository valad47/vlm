mkdir(".vlm-tmp")

local exists, pkgremotes = pcall(require, if _WIN32 then `{home}/.vlm/vlm-remotes` else "/etc/vlm-remotes")
if not exists then pkgremotes = {} end
local function install_remote(v)
    if not pkgremotes[v] then
        return false
    end
    execute("git", "clone", "-q", "--recursive", "--depth", "1", pkgremotes[v], `.vlm-tmp/{v}`)
    cd(`.vlm-tmp/{v}`)
    execute("vlm", "build")
    local deppkg = require(`build/pkginfo`)
    execute("vlm", "install", `{deppkg.pkgname}-{deppkg.pkgver}.vlp`)
    cd("../..")
    return true
end

local exists = pcall(execute, "test", "-f", filename)
if not exists then
    local err = install_remote(filename)

    execute("rm", "-rf", ".vlm-tmp")
    if not err then
        error(`Unknown file or module [{filename}]`)
    end
else

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
    if _WIN32 then

    execute(
        "rmdir",
        "/s",
        "/q",
        pkgdir
    )
    execute(
    	"mkdir",
    	pkgdir
    )
    execute(
    	"xcopy",
    	".vlm-tmp/build",
    	pkgdir,
    	"/E",
    	"/I",
    	"Y"
    )
    else

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

    end

    if pkginfo.dependency then
        for i,v in pkginfo.dependency do
            local exists = pcall(execute, "test", "-d", `{home}/.vlm/{v}`)
            if exists then continue end

            if not install_remote(v) then
                print(`[WARNING] you don't have module [{v}] installed, which is required by module [{pkgname}]`)
                continue
            end
        end
   end
end)

if _WIN32 then
	execute("rmdir", ".vlm-tmp")
else
	execute("rm", "-rf", ".vlm-tmp")
end
if err then
    error(err)
end
print(`Succesfully installed package [{pkgname}]`)
end
