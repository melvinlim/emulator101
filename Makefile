
CC = gcc
CFLAGS = -Wall -g

LIBS = -lncurses

# Define the executable name
TARGET1 = invaders
TARGET2 = wolf

# Define the source files
SRCS1 = 8080emu.c spaceinvaders.c
SRCS2 = 8080emu.c seawolf.c

# Define the object files (derived from source files)
OBJS1 = $(SRCS1:.c=.o)
OBJS2 = $(SRCS2:.c=.o)

# Default target: builds the executable
all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJS1)
	$(CC) $(CFLAGS) -o $@ $(OBJS1) $(LIBS)
$(TARGET2): $(OBJS2)
	$(CC) $(CFLAGS) -o $@ $(OBJS2) $(LIBS)

# Rule for compiling .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

# Clean target: removes generated files
clean:
	rm -f $(TARGET1) $(TARGET2) $(OBJS1) $(OBJS2)
