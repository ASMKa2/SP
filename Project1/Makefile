CC = gcc
CFLAGS = -Wall
TARGET = 20181592
OBJECTS = 20181592.o shell.o memory.o opcode.o

$(TARGET) : $(OBJECTS)
			  $(CC) $(CFLAGS) -o $@ $(OBJECTS) -lm

clean: 
	rm *.o 
	rm $(TARGET)
