# Compiler and Flags
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -O2 -Iinclude -Iexternal/include
LDFLAGS := -Lexternal/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Directories
SRC_DIRS := src src/core src/devices src/frontend
BUILD_DIR := build

# Find all .c files in the specified directories
SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
# Convert .c file paths to .o file paths in the build directory
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)

TARGET := talea_system

all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compile C files to Object files
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

deps:
	./setup_dependencies.sh

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean