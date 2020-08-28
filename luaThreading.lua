
LUATHREAD_INTERNAL_FUNCTION = function()
    io.stderr:write("A function must be set first")
end

launchThread = function(func, ...)
    local args = table.pack(...);
    if args.n == 0 then
        LUATHREAD_INTERNAL_FUNCTION = func
    else
        LUATHREAD_INTERNAL_FUNCTION = function() func(table.unpack(args)) end
    end
    return _launchThread("LUATHREAD_INTERNAL_FUNCTION")
end

