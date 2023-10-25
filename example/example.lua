#!/usr/local/bin/lua
local luaThreading = require "luaThreading"

----- main function with explaination on how to use luaThreading -----

main = function()
    -- Launching the function myprint in a thread
    local thread = luaThreading.launchThread(myprint)
    
    -- Waiting for the thread to be finished
    luaThreading.joinThread(thread)

    -- Launching multiple threads in the same time
    local threads = {}
    for i=1,5 do
        threads[i] = luaThreading.launchThread(myprint)
    end
    
    -- Joining the multiples threads
    for i=1,5 do
        luaThreading.joinThread(threads[i])
    end

    -- Launching a function with arguments in a thread
    thread = luaThreading.launchThread(mysum, 6, 7, 10)

    -- Retrieving the result of a function launched in a thread
    local result = luaThreading.joinThread(thread)
    print(result)

    -- Performing operations on tables in threads
    local arr = {1, 2, 3}
    thread = luaThreading.launchThread(mytableop, arr)
    print(thread)
    arr = luaThreading.joinThread(thread)
    thread = luaThreading.launchThread(mytableop, arr)
    luaThreading.joinThread(thread)
end

----- Various functions called by the main one -----

myprint = function()
    print("Hello, world!")
    mysillysleep()
end

mysillysleep = function()
    local i = 0
    while i<50000000 do
        i = i+1
    end
end

mysum = function(...)
    local args = table.pack(...)
    io.stdout:write("Summing ")
    for i=1,args.n do
        io.stdout:write(args[i])
        if i ~= args.n then
            io.stdout:write("+")
        else
            io.stdout:write("\n")
        end
    end
    local ret = 0
    for i=1,args.n do
        ret = ret+args[i]
    end
    return ret
end

-- Prints the elements from the array and return an array where they are doubled.
mytableop = function(arr)
    local ret = {}
    for i=1,#arr do
        print(arr[i])
        ret[i] = arr[i] * 2
    end
    return ret
end

main()

