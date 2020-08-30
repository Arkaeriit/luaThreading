
LUATHREAD_INTERNAL_FUNCTION = function()
    io.stderr:write("A function must be set first")
end

launchThread = function(func, ...)
    local args = table.pack(...)
    if args.n == 0 then
        LUATHREAD_INTERNAL_FUNCTION = function() return func() end
    else
        LUATHREAD_INTERNAL_FUNCTION = function() return func(table.unpack(args)) end
    end
    return _launchThread("LUATHREAD_INTERNAL_FUNCTION")
end

LUATHREAD_TABLE_DETAIL = function(tab)
    local i = 0
    local ret = {}
    for k,v in pairs(tab) do
        i = i+1
        ret[i] = {}
        ret[i].key = k
        ret[i].value = v
    end
    ret[0] = i
    return ret
end

