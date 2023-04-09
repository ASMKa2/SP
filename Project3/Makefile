CC = gcc
CFLAGS = -Wall
TARGET = 20181592.out
OBJECTS = 20181592.o shell.o memory.o opcode.o assemble.o load.o breakpoint.o run.o

$(TARGET) : $(OBJECTS)
			  $(CC) $(CFLAGS) -o $@ $(OBJECTS) -lm

clean: 
	rm *.o 
	rm $(TARGET)
