#8080:
#	gcc -g spaceinvaders.c -lncurses

#seawolf:
#	gcc -g seawolf.c -lncurses -o wolf

CC = gcc
CFLAGS = -Wall -g

LIBS = -lncurses

# Define the executable name
TARGET = invaders

# Define the source files
SRCS = 8080emu.c spaceinvaders.c

# Define the object files (derived from source files)
OBJS = $(SRCS:.c=.o)

# Default target: builds the executable
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

# Rule for compiling .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

# Clean target: removes generated files
clean:
	rm -f $(TARGET) $(OBJS)
