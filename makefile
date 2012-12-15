SOURCES=main.c linkedlist.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=traffic

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	cc $(OBJECTS) -o $@

clean:
	rm -rf *.o $(EXECUTABLE)