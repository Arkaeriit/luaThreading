LUA = -llua -lm -ldl 
FLAGS = -Wall -Werror -g
LUATHREAD = -lluaThreading

all : example.bin

example.bin : example.o
	gcc example.o $(LUATHREAD) $(FLAGS) $(LUA) -o example.bin

example.o : example.c
	gcc -c example.c $(FLAGS) -o example.o

clean :
	rm -f *.bin
	rm -f *.o

