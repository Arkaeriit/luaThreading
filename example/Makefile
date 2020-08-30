FLAGS = -Wall -Werror -g
LUATHREAD = -lluaThreading
LUA = -llua -lm -ldl 

all : example.bin

example.bin : example.o
	gcc example.o $(LUATHREAD) $(FLAGS) $(LUA) -o example.bin

example.o : example.c
	gcc -c example.c $(FLAGS) -o example.o

clean :
	rm -f *.bin
	rm -f *.o

