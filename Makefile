# Define the compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -fplan9-extensions -std=c23
AR = ar
ARFLAGS = rcs

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
LIB_DIR = lib

# Output
LIB_NAME = pesticide.a

# Find all source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Default target
all: $(LIB_DIR)/$(LIB_NAME)

# Create the library directory if it doesn't exist
$(LIB_DIR):
	mkdir -p $(LIB_DIR)

# Create the build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the library
$(LIB_DIR)/$(LIB_NAME): $(OBJ_FILES) | $(LIB_DIR)
	$(AR) $(ARFLAGS) $@ $^

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build and lib directories
clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR)/$(LIB_NAME)

.PHONY: all clean
