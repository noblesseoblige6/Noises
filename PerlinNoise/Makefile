CC = g++
TARGET = a
EXCLUDES = 

SRCS = $(wildcard *.cpp)
SRCS := $(filter-out $(wildcard $(EXCLUDES)), $(SRCS))
OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
			$(CC) -o $@ $^
