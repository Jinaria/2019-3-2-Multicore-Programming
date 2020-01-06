# Compiler and Compile options.
CC = g++
CXXFLAGS = -Wall -g -std=c++11 -pthread -O3

# Macros specifying path for compile.
SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:.cpp=.o)
INC = ./include/
LIB = ./lib/

# Pre-Processor.
CXXFLAGS += -I$(INC) -L$(LIB)


TARGET = run


all: $(TARGET)

$(TARGET): $(OBJS) 
	$(CC) -o $(TARGET) $(OBJS) $(CXXFLAGS)



# Delete binary & object files
clean:
	$(RM) $(TARGET) $(OBJS)
