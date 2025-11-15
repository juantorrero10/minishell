
# -------------- Deifiniciones -----------------
TARGET = minishell
CC = gcc
DBG = gdb

INC_DIR=include
LIB_DIR=lib
OBJ_DIR=build/obj
BIN_DIR=build/bin
SRC_DIR=src

# archivos .c fuente
SRC = main.c read.c prompt.c init.c env.c signals.c command/execute.c command/builtin.c command/job.c

# archivos .a en $LIB_DIR sin prefijo lib ni extensión .a
LIB = parser64

LIB_FLAGS = $(addprefix -l, $(LIB))

OBJ=$(SRC:%.c=$(OBJ_DIR)/%.o)
BIN=$(BIN_DIR)/$(TARGET)

CCFLAGS = -Wall -Werror -Wextra -g -m64 -std=c17 -I$(INC_DIR)

# modo debug
ifdef DEBUG
CCFLAGS += -D__DEBUG
endif

# address sanitizer
ifdef ASAN
CCFLAGS += -fsanitize=address,undefined
endif

# --------------- COLORES -------------------------
WHITE  = \033[0;39m
GREEN  = \033[0;92m
PURPLE = \033[0;95m
CYAN   = \033[0;96m
RED    = \033[0;91m

# --------------- TAREAS -------------------------
all:$(BIN)

# Compilar (*.c -> *.o)
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c | $(OBJ_DIR) $(OBJ_DIR)/command
	@echo "$(PURPLE)--------------------------------------------------------------------------"
	@echo "	Compiling: $(CYAN)$< $(PURPLE) -> $(RED) $@ $(WHITE)"
	$(CC) $(CCFLAGS) -c $< -o $@

# Vincular (*.o -> binary)
$(BIN) : $(OBJ) | $(BIN_DIR)
	@echo "$(CYAN)--------------------------------------------------------------------------"
	@echo "	Linking: $(RED)$< $(CYAN)-> $(GREEN)$@$(WHITE)"
	$(CC) $(CCFLAGS) -o $@ $(OBJ) -L$(LIB_DIR) $(LIB_FLAGS)
	@echo " "

# Crear carpetas
$(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/command: | $(OBJ_DIR)
	mkdir -p $@

clean:
	@echo "$(RED)Removing .o files$(WHITE)"
	@rm -rf $(OBJ_DIR)

debug: all
	$(DBG) ./$(BIN)

.PHONY: all clean debug

