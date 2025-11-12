
# -------------- DEFINITIONS -----------------
TARGET = minishell
CC = gcc
DBG = gdb

INC_DIR=include
LIB_DIR=lib
OBJ_DIR=build/obj
BIN_DIR=build/bin
SRC_DIR=src

# .c files are especified here
SRC = main.c read.c prompt.c init.c env.c signals.c command/execute.c command/builtin.c command/job.c

# .a files inside ./$(LIB_DIR) no 'lib-' prefix nor '.a' extension
LIB = parser64

LIB_FLAGS = $(addprefix -l, $(LIB))

OBJ=$(SRC:%.c=$(OBJ_DIR)/%.o)
BIN=$(BIN_DIR)/$(TARGET)

CCFLAGS = -Wall -Werror -Wextra -g -m64 -std=c17 -I$(INC_DIR)

ifdef DEBUG
CCFLAGS += -D__DEBUG
endif


# --------------- COLORS -------------------------
WHITE  = \033[0;39m
GREEN  = \033[0;92m
PURPLE = \033[0;95m
CYAN   = \033[0;96m
RED    = \033[0;91m

# --------------- TASKS -------------------------
all:$(BIN)

# Compile (*.c -> *.o)
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c | $(OBJ_DIR) $(OBJ_DIR)/command
	@echo "$(PURPLE)--------------------------------------------------------------------------"
	@echo "	Compiling: $(CYAN)$< $(PURPLE) -> $(RED) $@ $(WHITE)"
	$(CC) $(CCFLAGS) -c $< -o $@

# Linking (*.o -> binary)
$(BIN) : $(OBJ) | $(BIN_DIR)
	@echo "$(CYAN)--------------------------------------------------------------------------"
	@echo "	Linking: $(RED)$< $(CYAN)-> $(GREEN)$@$(WHITE)"
	$(CC) $(CCFLAGS) -o $@ $(OBJ) -L$(LIB_DIR) $(LIB_FLAGS)
	@echo " "

# Directories
$(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/command: | $(OBJ_DIR)
	mkdir -p $@

clean:
	@echo "$(RED)Removing .o files$(WHITE)"
	@rm -rf $(OBJ_DIR)

run: all
	@echo "$(GREEN)$(BIN):$(WHITE)"
	@./$(BIN)

debug: all
	$(DBG) ./$(BIN)

.PHONY: all run clean debug

