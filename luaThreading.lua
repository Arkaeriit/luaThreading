
LUATHREAD_GLOBAL_NAME.LUATHREAD_INTERNAL_FUNCTION = function()
    io.stderr:write("A function must be set first")
end

LUATHREAD_GLOBAL_NAME.launchThread = function(func, ...)
    local args = table.pack(...)
    if args.n == 0 then
        LUATHREAD_GLOBAL_NAME.LUATHREAD_INTERNAL_FUNCTION = function() return func() end
    else
        LUATHREAD_GLOBAL_NAME.LUATHREAD_INTERNAL_FUNCTION = function() return func(table.unpack(args)) end
    end
    return LUATHREAD_GLOBAL_NAME._launchThread()
end

LUATHREAD_GLOBAL_NAME.LUATHREAD_TABLE_DETAIL = function(tab)
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

