# Lua Threading

This library lets you easily do multi-threading in Lua.

# User manual

## Example

```lua
local luathreading = require "luathreading"
--launch the function myfunc with myarg as the only argument
local thread = luathreading.launch_thread(myfunc, myarg)
--wait for the thread to end and get the return value of myfunc
local result = thread:join()
```

A more detailed example is to be found in the example folder of this repo, under the name example.lua.

## API

### Threads

Threads are created with the function `launch_thread`. This function makes a new Lua thread in the Lua state and executes it in a separate POSIX thread. The threads starts to run when it is created.

The function `launch_thread` takes as first argument the function that will be ran in the new thread and any additional arguments will be given to the function being launched.

To wait for a thread to be finished and get it's return value, you should use the `join` method of the thread. The `join` method returns the return value of the launched function. If the launched function does not return anything, the `join` method will return `nil`. If the launched function returns multiple values, only the last value will be returned. If you really need to return multiple arguments in a function, you should pack them in a table.

When the handle to a thread is lost, the thread will still continue to run. But generating threads without joining them will create a memory leak.

When giving values as argument to a new thread or getting return values from a thread, nil, number, booleans, strings, light userdata, and C functions are exchanged by value while lua functions, threads, tables, and userdata are exchanged by reference.

### Mutexes

To synchronize threads or protecting resources used by multiple threads, Lua Threading can generate mutexes with the `new_mutex` function. Those mutexes have `lock` and `unlock` method and are generated in the unlocked state.

As there is not a lot of fine control on memory in Lua, values used by multiple threads should always be protected by mutexes. Values given as argument to thread by reference such as table should be protected if they are used in the child thread and the parent thread at the same time.

Mutexes are garbage collected so using them should not cause memory leak.

## Installation

To install Lua Threading you must have Lua installed in your system. If so, simply run `make && make install` from the root of this repository and the library will be installed. You can test it by going into the example folder and running `./example.lua`.

