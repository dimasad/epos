
CC = x86_64-w64-mingw32-gcc
CFLAGS = -std=gnu11
DEFINE = -DDEBUG
HEADERS = epos2.h


epos2cmd.exe: epos2cmd.o epos2.o
	$(CC) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(DEFINE) -o $@ -c $<


clean:
	rm -f *.o *.exe
