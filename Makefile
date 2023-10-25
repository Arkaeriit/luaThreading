FLAGS = -Wall -Werror -g
PT = -lpthread
LIBLUAPATH_5_3 = /usr/local/lib/lua/5.3
LIBLUAPATH_5_4 = /usr/local/lib/lua/5.4

all : luathreading.so

luathreading.o : luaThreading.c
	gcc -c -fPIC $< $(FLAGS) -o $@

luathreading.so : luathreading.o
	gcc -shared -g $< $(PT) -o $@

install : 
	mkdir -p $(LIBLUAPATH_5_3) $(LIBLUAPATH_5_4)
	cp -f luathreading.so $(LIBLUAPATH_5_4)/luathreading.so
	cp -f luathreading.so $(LIBLUAPATH_5_3)/luathreading.so

uninstall :
	rm -f $(LIBLUAPATH_5_4)/luathreading.so
	rm -f $(LIBLUAPATH_5_3)/luathreading.so

clean :
	rm -f *.o
	rm -f *.so

