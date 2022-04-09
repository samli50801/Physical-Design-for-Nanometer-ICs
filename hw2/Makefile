CC=g++
LDFLAGS=-std=c++11 -O3 -lm
SOURCES=src/btree.cpp src/floorplanner.cpp src/main.cpp
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=fp
INCLUDES=src/module.h src/btree.h src/floorplanner.h

all: $(SOURCES) $(EXECUTABLE)
	cp $(EXECUTABLE) bin/

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o:  %.c  ${INCLUDES}
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE) bin/$(EXECUTABLE)
