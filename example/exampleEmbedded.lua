
----- main function with explaination on how to use luaThreading -----

main = function()
    --launching the function myprint in a thread
    local thread = luaThreading.launchThread(myprint)
    
    --waiting for the thread to be finished
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

