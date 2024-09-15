# Define variables
BUILD ?= Debug
CMAKEFLAGS := -DCMAKE_BUILD_TYPE=${BUILD} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
BUILD_DIR := build
BIN_DIR := bin
NAME := clox

# Targets
build: cmake_build

run: build
	@./$(BIN_DIR)/$(NAME)

# Configure CMake
cmake_configure:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKEFLAGS) ..

# Build executable using CMake
cmake_build: cmake_configure
	@cd $(BUILD_DIR) && make

# Clean build and bin directories
clean:
	@rm -rf $(BUILD_DIR) $(BIN_DIR)

format:
	@echo "Formattting code..."
	@find . -iname '*.h' -o -iname '*.c' | xargs clang-format -i
	@echo "Done!"

.PHONY: cmake_configure cmake_build test clean format