CPPC=g++ -I/usr/include/Poco
CPPFLAGS=-std=c++11 -pthread
CPPLDFLAGS=
LDLIBS= -L/usr/lib -lPocoNet

CC=gcc
CFLAGS=-std=c99 -D_SVID_SOURCE -D_POSIX_SOURCE -D_BSD_SOURCE -Wall
LDFLAGS=-lpthread

EXEC=device launcher server

all: $(EXEC)

device: device.o
	$(CC) -o $@ $^ $(LDFLAGS)

launcher: devices_launcher.o
	$(CC) -o $@ $^ $(LDFLAGS)

server: server.o devices_counters.o devices_counters.h
	$(CPPC) -o $@ $^ $(CPPFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

%.o: %.cpp
	$(CPPC) -o $@ -c $< $(CPPFLAGS) $(LDLIBS)

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
