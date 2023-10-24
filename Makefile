FLAGS = -Wall -Werror -g
PT = -lpthread
LIBPATH = /usr/lib64
LIBLUAPATH_5_3 = /usr/local/lib/lua/5.3
LIBLUAPATH_5_4 = /usr/local/lib/lua/5.4
INCLUDEPATH = /usr/include

all : libluaThreading.so

luaThreading.o : luaThreading.c luaThreading.h
	gcc -c -fPIC luaThreading.c $(FLAGS) -o luaThreading.o

libluaThreading.so : luaThreading.o
	gcc -shared -g luaThreading.o $(PT) -o libluaThreading.so

install : 
	mkdir -p $(LIBPATH)/luaThreading $(INCLUDEPATH) $(LIBLUAPATH_5_3) $(LIBLUAPATH_5_4)
	cp -f libluaThreading.so $(LIBPATH)/libluaThreading.so
	cp -f luaThreading.h $(INCLUDEPATH)/luaThreading.h
	cp -f libluaThreading.so $(LIBLUAPATH_5_4)/luaThreading.so
	cp -f libluaThreading.so $(LIBLUAPATH_5_3)/luaThreading.so

uninstall :
	rm -rf $(LIBPATH)/luaThreading
	rm -f $(LIBPATH)/libluaThreading.so
	rm -f $(INCLUDEPATH)/luaThreading.h
	rm -f $(LIBLUAPATH)/luaThreading.so

clean :
	rm -f *.o
	rm -f *.so

