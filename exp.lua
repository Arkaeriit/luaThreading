
myprint = function()
    print("Hello, world!")
    os.execute("sleep 1")
end

mynewprint = function(str, str2)
    print(str)
    if(str2) then
        print(str2)
        return 2,77
    else
        return 1,99
    end
end

identity = function(arg)
    return arg
end

table.compare = function(taba, tabb)
    subcompare = function(tab1, tab2)
        for k,v in pairs(tab1) do
            if type(v) == "table" and type(tab2[k]) == "table" then
                if not table.compare(v, tab2[k]) then
                    return false
                end
            else
                if v ~= tab2[k] then
                    return false
                end
            end
        end
        return true
    end
    return subcompare(taba,tabb) and subcompare(tabb,taba)
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
    print(type(thread), thread)
    print(joinThread(thread))
    for k,v in pairs({"str", 42, false, 6.6, nil, _launchThread, {"str", 55, {1, 2, 3}}} ) do
        local thread = launchThread(identity, v)
        local res = joinThread(thread)
        if(v ~= res) and (type(v) == "table" and not table.compare(v,res)) then
            print("Error in copy at elem "..tostring(k))
        end
    end
end

