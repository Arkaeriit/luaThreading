LUA = -llua -lm -ldl 
FLAGS = -Wall -Werror -g
PT = -lpthread

all : exampleEmbedded.bin cleanCopy

exampleEmbedded.bin : exampleEmbedded.o luaThreading.o
	gcc exampleEmbedded.o luaThreading.o $(FLAGS) $(LUA) $(PT) -o exampleEmbedded.bin

exampleEmbedded.o : exampleEmbedded.c
	gcc -c exampleEmbedded.c $(FLAGS) -o exampleEmbedded.o

luaThreading.o : copy
	gcc -c luaThreading.c $(FLAGS) -o luaThreading.o

copy :
	cp ../luaThreading.c .
	cp ../luaThreading.h .
	cp ../luaThreading.lua .

cleanCopy :
	rm -f luaThreading.c
	rm -f luaThreading.h
	rm -f luaThreading.o

clean :
	rm -f *.bin
	rm -f luaThreading.lua
	rm -f *.o

