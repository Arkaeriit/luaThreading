FLAGS = -Wall -Werror -g
LUA = -llua -lm -ldl 
PT = -lpthread

all : test.bin

test.bin : test.o luaStateGen.o luaThreading.o
	gcc test.o luaStateGen.o luaThreading.o $(FLAGS) $(LUA) $(PT) -o test.bin

test.o : test.c
	gcc -c test.c $(FLAGS) -o test.o

luaStateGen.o : luaStateGen.c luaStateGen.h
	gcc -c luaStateGen.c $(FLAGS) -o luaStateGen.o

luaThreading.o : luaThreading.c luaThreading.h
	gcc -c luaThreading.c $(FLAGS) -o luaThreading.o

clean :
	rm -f *.o
	rm -f *.bin

