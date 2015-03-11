CC  = g++
TARGET  = a 
SRCS    = $(wildcard *.cpp)
OBJS    = $(SRCS:.cpp=.o)
	 
$(TARGET): $(OBJS)
	    $(CC) -o $@ $^
