BUILD=bin

CC=g++
CPPFLAGS=-I./

all: lib-properties.a

lib-properties.a: LibProperties.o
	ar cr lib-properties.a LibProperties.o

LibProperties.o: LibProperties/LibProperties.cpp
	$(CC) -c LibProperties/LibProperties.cpp

test: test.o lib-properties.a
	$(CC) $^ -o $@ -pthread -lrt

test.o: Test/main.cpp
	$(CC) $(CPPFLAGS) -c $^ -o $@

clean:
	rm -f LibProperties.o lib-properties.a test config.properties

.PHONY: all clean
