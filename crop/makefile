# Compiler
CC = mipsel-linux-gnu-gcc

# Compiler Flags
CFLAGS = -march=mips32 -EL -ffreestanding -nostdlib -nodefaultlibs -fomit-frame-pointer -fno-stack-protector -Os -mno-abicalls -fno-reorder-blocks -fno-optimize-sibling-calls

# Output Executable
OUTPUT = crop.bin

# Find all C source files
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

# Linker Flags
# Define the fixed-address function symbol `console` so C can call it as a normal extern:
#   extern void console(void *, int, int);
# mapping, adjust accordingly.
LDFLAGS = -Wl,--defsym,console=0x080160,--defsym,consoleD=0x080c60,--defsym,setdate=0x0693a0

# Default target
all: $(OUTPUT)

# Link the object files into an executable
$(OUTPUT): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

# Compile each .c file into .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJS) $(OUTPUT)
