### compile option ###
CC = g++
CFLAGS = -std=c++11 -Wall -Werror -pedantic -ggdb3 
DFLAGS = -MMD # generate foo.d for each foo.cpp 
LIB = -lpthread

### variables ###
PROGS = http-caching-proxy
MAIN_OBJS = $(patsubst %, %.o, $(PROGS))
SRCS = $(wildcard *.cpp) $(wildcard */*.cpp)
OBJS = $(filter-out $(MAIN_OBJS), $(patsubst %.cpp, %.o, $(notdir $(SRCS))))
# $(info    PROGS is $(PROGS)) # print variable
# $(info    OBJS is $(OBJS)) # print variable

### task/target ###
all: $(PROGS)

$(PROGS): %: %.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIB)

VPATH = socket proxy-server # search subdirectories
%.o: %.cpp
	$(CC) $(CFLAGS) $(DFLAGS) -c $<

-include $(SRCS:%.cpp=%.d) # include all dependencies

.PHNOY: clean
clean:
	rm -f *~ *.o *.d $(PROGS)