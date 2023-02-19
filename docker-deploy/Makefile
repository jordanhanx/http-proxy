### compile option ###
CC = g++
CFLAGS = -std=c++11 -Wall -Werror -pedantic -ggdb3 -MMD
LIB = -lpthread

### directory ###
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
# TEST_DIR =

$(shell mkdir -p $(BUILD_DIR))
$(shell mkdir -p $(OBJ_DIR))

### variables ###
MAIN_SRCS = $(wildcard $(SRC_DIR)/*.cpp)
SRCS = $(wildcard $(SRC_DIR)/*/*.cpp)
MAIN_OBJS = $(addprefix $(OBJ_DIR)/, $(patsubst %.cpp, %.o, $(notdir $(MAIN_SRCS))))
OBJS = $(addprefix $(OBJ_DIR)/, $(patsubst %.cpp, %.o, $(notdir $(SRCS))))
PROGS = $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp, %, $(notdir $(MAIN_SRCS))))
# $(info    MAIN_SRCS is $(MAIN_SRCS)) # print variable
# $(info    SRCS is $(SRCS)) # print variable
# $(info    MAIN_OBJS is $(MAIN_OBJS)) # print variable
# $(info    OBJS is $(OBJS)) # print variable
# $(info    PROGS is $(PROGS)) # print variable

### targets ###
all: $(PROGS)
# @echo running: $@ : $^ # print taget and dependencies

$(PROGS): $(BUILD_DIR)/%: $(OBJ_DIR)/%.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIB)

$(MAIN_OBJS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.SECONDEXPANSION:
$(OBJS) : $(OBJ_DIR)/%.o: $$(SRC_DIR)/$$*/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

-include $(wildcard $(OBJ_DIR)/*.d) # include all dependencies

.PHNOY:
clean:
	rm -rf *~ *.o *.d $(BUILD_DIR)/

