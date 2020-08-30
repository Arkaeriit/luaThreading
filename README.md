# luaThreading
This library lets you easily do multithreading in Lua.

# User manual
## How to use it
This library adds two functions.
* `launchThread` which takes as argument a function, followed by this function's arguments and returns a thread object. 
* `joinThread` which takes a thread object an only argument and returns the lat return value of the function launched in the thread.

When creating a thread with launchThread, the Lua state is copied and the desired function is run in a new POSIX thread, from the newly generated Lua state. This means that functions launched in a new thread will not be able to modify the state of the main thread.

 ## Example
```lua
--launch the function myfunc with myarg as the only argument
local thread = launchThread(myfunc, myarg)
--wait for the thread to end and get the return value of myfunc
local result = joinThread(thread)
```
A more detailed example is to be found in the example folder of this repo

## Importing the library
As of now, the only way to import the library is when you are embedding Lua into a C program. In this case, you must have the file luaThreading.h included and you must run the function `lt_include` on your Lua state. Check example/example.c to see how it is done.

## Instalation
To install luaThread you must have Lua installed in your system. If so, simply run `make && make install` from the root of this repo and the library will be installed. You can test it by going into the example folder and running `make && ./example.bin`. If the command make in the example folder does not work, you might need to change the first line of the makefile to include the correct flags to link Lua in your system.

## Limitations
### Return types
* The function `joinThread` can not return any values lua functions can not be returned (C function can)
* userdata are treated as lightuserdata

### Internal function
LuaThreading relies on three global functions to operate, `_launchThread`, `LUATHREAD_TABLE_DETAIL` and `LUATHREAD_INTERNAL_FUNCTION`. If they are overridden, the library will not work.

# Roadmap
* Use a single global table instead of five global functions
* Make the library available from Lua directly without needing to embed Lua into C.
* Maybe let functions called from a separate thread return as many values as needed instead of a single one.

