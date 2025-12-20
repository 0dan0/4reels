# Compiler
CC = mipsel-linux-gnu-gcc

# Compiler Flags
CFLAGS = -march=mips32 -EL -ffreestanding -nostdlib -nodefaultlibs -fomit-frame-pointer -fno-stack-protector -Os -mno-abicalls -fno-reorder-blocks

# Output Executable
OUTPUT = manwb.bin

# Find all C source files
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

# Default target
all: $(OUTPUT)

# Link the object files into an executable
$(OUTPUT): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compile each .c file into .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJS) $(OUTPUT)
