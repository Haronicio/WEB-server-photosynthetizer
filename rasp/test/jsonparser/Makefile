# Compiler and compiler flags
CXX = gcc
CXXFLAGS = -Wall

# Target executable
TARGET = jsonparser

# Source files
SRCS = jsonparser.c cJSON.c

# Object files
OBJS = $(SRCS:.cpp=.o)

# Phony targets
.PHONY: all clean

# Default target
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Rule to build object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up object files and the executable
clean:
	rm -f $(OBJS) $(TARGET)