CFLAGS = -zI -I./include

# Directories
SRC_ROOT = src
OBJ_ROOT = obj

# Find all .c and .s files in any subdirectory of src
# This works for src/string, src/math, src/stdio, etc.
C_SRCS = $(shell find $(SRC_ROOT) -name "*.c")
S_SRCS = $(shell find $(SRC_ROOT) -name "*.s")

# Convert source paths to object paths
# Example: src/string/strlen.c -> obj/string/strlen.o
OBJS = $(C_SRCS:$(SRC_ROOT)/%.c=$(OBJ_ROOT)/%.o) \
       $(S_SRCS:$(SRC_ROOT)/%.s=$(OBJ_ROOT)/%.o)

LIB = libc.a

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) -rs $@ $(OBJS)

# Rule for C files (automatically creates subdirs in obj/)
$(OBJ_ROOT)/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule for Assembly files
$(OBJ_ROOT)/%.o: $(SRC_ROOT)/%.s
	@mkdir -p $(dir $@)
	$(AS) -o $@ $<

clean:
	rm -rf $(OBJ_ROOT) $(LIB)

.PHONY: all clean
