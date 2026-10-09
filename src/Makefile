
# -------------- Deifiniciones -----------------
TARGET = minishell
CC = gcc
DBG = gdb

INC_DIR=../include
LIB_DIR=../lib
OBJ_DIR=../build/obj
OBJ_THIS=../build/obj/mshmain
BIN_DIR=../build/bin
SRC_DIR=.

# archivos .c fuente
SRC = main.c read.c prompt.c init.c env.c signals.c command/execute.c command/builtin.c command/job.c

# archivos .a en $LIB_DIR sin prefijo lib ni extensión .a

LIB_FLAGS = $(addprefix -l, $(LIB))

OBJ=$(SRC:%.c=$(OBJ_THIS)/%.o)
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
all:$(OBJ)

# Compilar (*.c -> *.o)
$(OBJ_THIS)/%.o : $(SRC_DIR)/%.c | $(OBJ_THIS) $(OBJ_THIS)/command
	@echo "$(PURPLE)--------------------------------------------------------------------------"
	@echo "	Compiling: $(CYAN)$< $(PURPLE) -> $(RED) $@ $(WHITE)"
	$(CC) $(CCFLAGS) -c $< -o $@

# Crear carpetas
$(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

$(OBJ_THIS):	| $(OBJ_DIR)
	mkdir -p $@

$(OBJ_THIS)/command: | $(OBJ_THIS)
	mkdir -p $@

clean:
	@echo "$(RED)Removing .o files$(WHITE)"
	@rm -rf $(OBJ_THIS)

debug: all
	$(DBG) ./$(BIN)

.PHONY: all clean debug demo

