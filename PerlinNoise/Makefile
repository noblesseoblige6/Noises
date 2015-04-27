CC = g++
TARGET = a
EXCLUDES = PerlinNoise.1.0.cpp

SRCS = $(wildcard *.cpp)
SRCS := $(filter-out $(wildcard $(EXCLUDES)), $(SRCS))
OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
			$(CC) -o $@ $^
