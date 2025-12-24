local function github(user, repo)
      return `https://github.com/{user}/{repo}`
end

return {
    ["io"]             = github("valad47", "vlm-io"),
    ["task"]           = github("valad47", "task"),
    ["json"]           = github("valad47", "vlm-json"),
    ["network"]        = github("valad47", "vlm-network"),
    ["tgbot"]          = github("valad47", "vlm-tgbot"),
    ["xor"]            = github("valad47", "vlm-xor"),
    ["pprint"]         = github("valad47", "vlm-pprint"),
    ["cbor"]           = github("valad47", "vlm-cbor"),
    ["signal"]         = github("valad47", "signal"),
    ["color"]          = github("valad47", "vlm-color"),
    ["system"]         = github("valad47", "vlm-system"),
    ["ipc"             = github("valad47", "vlm-ipc"),
    ["ipc-server"]     = github("valad47", "vlm-ipc-server"),
    ["ipc-client"]     = github("valad47", "vlm-ipc-client"),
    ["uinput"]         = github("valad47", "vlm-uinput"),
    ["env"]            = github("valad47", "vlm-env"),
    ["vkeyboard"]      = github("valad47", "vlm-vkeyboard"),
    ["input-keycodes"] = github("valad47", "vlm-input-keycodes"),
    ["vmacro"]         = github("valad47", "vlm-vmacro"),
}
