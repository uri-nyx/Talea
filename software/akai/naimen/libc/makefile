CFLAGS = -zI -I./include -I$(SDK)

CFLAGS_MINI = $(CFLAGS) -D_AKAI_STDIO_NO_FLOAT_

# Directories
SRC_ROOT = src
OBJ_ROOT = obj
OBJ_MINI = objmin

# Find all .c and .s files in any subdirectory of src
# This works for src/string, src/math, src/stdio, etc.
C_SRCS = $(shell find $(SRC_ROOT) -name "*.c")
S_SRCS = $(shell find $(SRC_ROOT) -name "*.s")

# Convert source paths to object paths
# Example: src/string/strlen.c -> obj/string/strlen.o
OBJS_FULL = $(C_SRCS:$(SRC_ROOT)/%.c=$(OBJ_ROOT)/%.o) \
			$(S_SRCS:$(SRC_ROOT)/%.s=$(OBJ_ROOT)/%.o)

OBJS_MINI = $(C_SRCS:$(SRC_ROOT)/%.c=$(OBJ_MINI)/%.o) \
			$(S_SRCS:$(SRC_ROOT)/%.s=$(OBJ_MINI)/%.o)

LIB = libc.a
MINI = libcmin.a

all: $(LIB) $(MINI)

$(LIB): $(OBJS_FULL)
	$(AR) -rs $@ $(OBJS_FULL)

$(MINI): $(OBJS_MINI) 
	$(AR) -rs $@ $(OBJS_MINI)

# dont generate output, only dump the lcc tre
debug:
	echo "WIP"

# Rule for C files (automatically creates subdirs in obj/)
$(OBJ_ROOT)/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_MINI)/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_MINI) -c -o $@ $<

# Rule for Assembly files
$(OBJ_ROOT)/%.o: $(SRC_ROOT)/%.s
	@mkdir -p $(dir $@)
	$(AS) -o $@ $<

$(OBJ_MINI)/%.o: $(SRC_ROOT)/%.s
	@mkdir -p $(dir $@)
	$(AS) -o $@ $<


clean: 
	rm -rf $(OBJ_ROOT) $(OBJ_MINI) $(LIB) $(MINI)

.PHONY: all clean
