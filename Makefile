
# -------------- Deifiniciones -----------------
TARGET = minishell
CC = gcc
DBG = gdb

INC_DIR=include
LIB_DIR=lib
OBJ_DIR=build/obj
BIN_DIR=build/bin
SRC_DIR=src

# archivos .a en $LIB_DIR sin prefijo lib ni extensión .a
LIB = parser64

LIB_FLAGS = $(addprefix -l, $(LIB))

SRC_MAIN     := $(wildcard src/*.c)  \
                $(wildcard src/command/*.c)
SRC_PARSER   := $(wildcard src/parser/*.c)

OBJ_MAIN     := $(patsubst src/%.c,         $(OBJ_DIR)/mshmain/%.o, $(SRC_MAIN))
OBJ_PARSER   := $(patsubst src/parser/%.c,  $(OBJ_DIR)/parser/%.o,  $(SRC_PARSER))

OBJ := $(OBJ_MAIN) $(OBJ_PARSER)

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

# Vincular (*.o -> binary)
$(BIN): $(OBJ) | $(BIN_DIR) $(OBJ_DIR)
	@echo "$(CYAN)--------------------------------------------------------------------------"
	@echo "	Linking: $(RED)$< $(CYAN)-> $(GREEN)$@$(WHITE)"
	$(CC) $(CCFLAGS) -o $@ $(OBJ) -L$(LIB_DIR) $(LIB_FLAGS)
	@echo " "

# Build modules
$(OBJ):	| $(OBJ_DIR)
	$(MAKE) -C src
	$(MAKE) -C src/parser

# Crear carpetas
$(BIN_DIR):
	mkdir -p $@

# Call al stages of makefiles
$(OBJ_DIR):
	mkdir -p $@

clean:
	@echo "$(RED)Removing .o files$(WHITE)"
	@rm -rf $(OBJ_DIR)

debug: all
	$(DBG) ./$(BIN)

.PHONY: all clean debug

