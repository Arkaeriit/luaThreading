FLAGS = -Wall -Werror -g
LUA = -llua -lm -ldl 
PT = -lpthread
LIBPATH = /usr/lib64
INCLUDEPATH = /usr/include

all : libluaThreading.so luaThreading.luac

luaThreading.o : luaThreading.c luaThreading.h
	gcc -c -fPIC luaThreading.c $(FLAGS) -o luaThreading.o

libluaThreading.so : luaThreading.o
	gcc -shared  luaThreading.o $(PT) -o libluaThreading.so

luaThreading.luac : luaThreading.lua
	luac -o luaThreading.luac luaThreading.lua

install : 
	mkdir -p $(LIBPATH)/luaThreading
	cp -f luaThreading.luac $(LIBPATH)/luaThreading/luaThreading.luac
	cp -f libluaThreading.so $(LIBPATH)/libluaThreading.so
	cp -f luaThreading.h $(INCLUDEPATH)/luaThreading.h

uninstall :
	rm -rf $(LIBPATH)/luaThreading
	rm -f $(LIBPATH)/libluaThreading.so
	rm -f $(INCLUDEPATH)/luaThreading.h

clean :
	rm -f *.o
	rm -f *.luac
	rm -f *.so

