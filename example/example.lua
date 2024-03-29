#!/usr/local/bin/lua
local luathreading = require "luathreading"

----- main function with explaination on how to use luathreading -----

main = function()
    -- Launching the function myprint in a thread
    local thread = luathreading.launch_thread(myprint)
    
    -- Waiting for the thread to be finished
    thread:join()

    -- Launching multiple threads in the same time
    local threads = {}
    for i=1,5 do
        threads[i] = luathreading.launch_thread(myprint)
    end
    
    -- Joining the multiples threads
    for i=1,5 do
        threads[i]:join()
    end

    -- Launching a function with arguments in a thread
    thread = luathreading.launch_thread(mysum, 6, 7, 10)

    -- Retrieving the result of a function launched in a thread
    local result = thread:join()
    print(result)

    -- Performing operations on tables in threads
    local arr = {1, 2, 3}
    thread = luathreading.launch_thread(mytableop, arr)
    print(thread)
    arr = thread:join()
    thread = luathreading.launch_thread(mytableop, arr)
    thread:join()

    -- Synchronizing threads with a mutex
    local mutex = luathreading.new_mutex()
    for i=1,3 do
        threads[i] = luathreading.launch_thread(mysyncedfunc, i, mutex)
    end
    for i=1,3 do
        threads[i]:join()
    end
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

-- Locks a mutex and print some messages
mysyncedfunc = function(thead_name, mutex)
    mutex:lock()
    print("Starting thread "..tostring(thead_name))
    mysillysleep()
    print("Ending thread "..tostring(thead_name))
    mutex:unlock()
end

main()

