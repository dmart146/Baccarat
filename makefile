
# Simple Makefile for building baccarat
# Targets:
#   make        -> default: builds 'baccarat'
#   make run    -> builds and runs the program
#   make clean  -> remove build artifacts

CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2

SRCS = baccarat.cpp cards.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = baccarat

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

