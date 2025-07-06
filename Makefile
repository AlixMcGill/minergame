# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -std=c++17

# Files and directories
SRC := main.cpp
TARGET := build/main
LIBS := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Default rule
all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LIBS)

# Clean rule
clean:
	rm -rf build

