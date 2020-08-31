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
local luaThreading = require "luaThreading"
--launch the function myfunc with myarg as the only argument
local thread = luaThreading.launchThread(myfunc, myarg)
--wait for the thread to end and get the return value of myfunc
local result = luaThreading.joinThread(thread)
```
A more detailed example is to be found in the example folder of this repo, under the name example.lua.

## Importing the library
If you have installed it, you can import the library by doing `local luaThreading = require "luaThreading"`.

Alternatively, if you embed Lua into C, you can follow the makefile and the exampleEmbedded files in the example folder. On the other hand, I personally recommend the use of the require function.
	
## Installation
To install luaThread you must have Lua installed in your system. If so, simply run `make && make install` from the root of this repo and the library will be installed. You can test it by going into the example folder and running `make && ./example.bin`. If the command make in the example folder does not work, you might need to change the first line of the makefile to include the correct flags to link Lua in your system.

## Limitations
### Return types
* The function `joinThread` can not return any values lua functions can not be returned (C function can)
* userdata are treated as lightuserdata

### Internal function
The library relies on a global table, `LUATHREAD_GLOBAL_NAME`. If this object is overridden, the library might not work.

# Roadmap
* Maybe let functions called from a separate thread return as many values as needed instead of a single one.
* Get rid of the global table.

