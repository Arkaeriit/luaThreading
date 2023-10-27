CFLAGS += -Wall -Wextra
ifdef DEV
CFLAGS += -g -Werror
else
CFLAGS += -O2
endif

PT = -lpthread
LIBLUAPATH_5_3 = /usr/local/lib/lua/5.3
LIBLUAPATH_5_4 = /usr/local/lib/lua/5.4

all : luathreading.so

luathreading.o : luaThreading.c
	gcc -c -fPIC $< $(CFLAGS) -o $@

luathreading.so : luathreading.o
	gcc -shared $< $(CFLAGS) $(PT) -o $@

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

