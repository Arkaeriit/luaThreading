
myprint = function()
    print("Hello, world!")
    os.execute("sleep 1")
end

main = function()
    local threads = {}
    for i=1,4 do
        threads[i] = runFunc("myprint")
    end
    for i=1,4 do
        joinFunc(threads[i])
    end
end

