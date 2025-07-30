local function github(user, repo)
      return `https://github.com/{user}/{repo}`
end

return {
	["io"]           = github("valad47", "vlm-io"),
	["task"]         = github("valad47", "task"),
	["json"]         = github("valad47", "vlm-json"),
	["network"]      = github("valad47", "vlm-network"),
	["tgbot"]        = github("valad47", "vlm-tgbot"),
    ["xor"]          = github("valad47", "vlm-xor"),
    ["pprint"]       = github("valad47", "vlm-pprint"),
    ["cbor"]         = github("valad47", "vlm-cbor"),
    ["signal"]       = github("valad47", "signal"),
    ["color"]        = github("valad47", "vlm-color"),
}
