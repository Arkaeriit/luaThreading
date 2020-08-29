
myprint = function()
    print("Hello, world!")
    os.execute("sleep 1")
end

mynewprint = function(str, str2)
    print(str)
    if(str2) then
        print(str2)
        return 2
    else
        return 1
    end
end

main = function()
    local threads = {}
    for i=1,4 do
        threads[i] = launchThread(myprint)
    end
    for i=1,4 do
        joinThread(threads[i])
    end
    do
        local var = 5
        local locfnc = function()
            print("local", var)
        end
        globfnc = function()
            locfnc()
        end
        locfnc()
        local t = launchThread(globfnc)
        joinThread(t)
    end
    globfnc()
    local thread = launchThread(mynewprint, "welcome back", "we missed you")
    print(joinThread(thread))
end

