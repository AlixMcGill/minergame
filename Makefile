
# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -std=c++17 -Isrc

# Directories
SRC_DIR := src
BUILD_DIR := build

# Source and object files
SRCS := $(shell find $(SRC_DIR) -name "*.cpp")
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

TARGET := $(BUILD_DIR)/main
LIBS := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Default rule
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Build .o files from .cpp
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -rf $(BUILD_DIR)

