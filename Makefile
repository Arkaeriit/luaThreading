FLAGS = -Wall -Werror -g
PT = -lpthread
LIBPATH = /usr/lib64
LIBLUAPATH_5_3 = /usr/local/lib/lua/5.3
LIBLUAPATH_5_4 = /usr/local/lib/lua/5.4
INCLUDEPATH = /usr/include

all : libluathreading.so

luathreading.o : luaThreading.c luaThreading.h
	gcc -c -fPIC $< $(FLAGS) -o $@

libluathreading.so : luathreading.o
	gcc -shared -g $< $(PT) -o $@

install : 
	mkdir -p $(LIBPATH)/luaThreading $(INCLUDEPATH) $(LIBLUAPATH_5_3) $(LIBLUAPATH_5_4)
	cp -f libluathreading.so $(LIBPATH)/libluathreading.so
	cp -f luaThreading.h $(INCLUDEPATH)/luaThreading.h
	cp -f libluathreading.so $(LIBLUAPATH_5_4)/luathreading.so
	cp -f libluathreading.so $(LIBLUAPATH_5_3)/luathreading.so

uninstall :
	rm -rf $(LIBPATH)/luaThreading
	rm -f $(LIBPATH)/libluathreading.so
	rm -f $(INCLUDEPATH)/luaThreading.h
	rm -f $(LIBLUAPATH)/luathreading.so

clean :
	rm -f *.o
	rm -f *.so

