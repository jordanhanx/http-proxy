### compile option ###
CC = g++
CFLAGS = -std=c++11 -Wall -Werror -pedantic -O3 -MMD $(MTHREAD_FLAG) $(INCLUDE_DIR)

MTHREAD_FLAG = -pthread
INCLUDE_DIR = -I /usr/include/boost

### directory ###
SRC_DIR = cpp_src
BUILD_DIR = cpp_build
OBJ_DIR = $(BUILD_DIR)/obj


### variables ###
MAIN_SRCS = $(wildcard $(SRC_DIR)/*.cpp)
SRCS = $(wildcard $(SRC_DIR)/*/*.cpp)
MAIN_OBJS = $(addprefix $(OBJ_DIR)/, $(patsubst %.cpp, %.o, $(notdir $(MAIN_SRCS))))
OBJS = $(addprefix $(OBJ_DIR)/, $(patsubst %.cpp, %.o, $(notdir $(SRCS))))
PROGS = $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp, %, $(notdir $(MAIN_SRCS))))

### targets ###
all: initProj $(PROGS)
	@echo Compiled successfully.

$(PROGS): $(BUILD_DIR)/%: $(OBJ_DIR)/%.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(MAIN_OBJS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.SECONDEXPANSION:
$(OBJS) : $(OBJ_DIR)/%.o: $$(SRC_DIR)/$$*/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

initProj: $(SRC_DIR) $(BUILD_DIR) $(OBJ_DIR)
$(SRC_DIR):
	mkdir -p $(SRC_DIR)
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

-include $(wildcard $(OBJ_DIR)/*.d) # include all dependencies

.PHNOY:
clean:
	rm -rf *~ *.o *.d $(BUILD_DIR)/

