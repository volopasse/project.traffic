SOURCES=traffic.c linkedlist.c output.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=traffic

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	cc $(OBJECTS) -o $@

clean:
	rm -rf *.o $(EXECUTABLE)