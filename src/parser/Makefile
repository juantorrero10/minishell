
# -------------- Deifiniciones -----------------
TARGET = parserdemo
CC = gcc
DBG = gdb

INC_DIR=../../include/parser
LIB_DIR=../../lib
OBJ_DIR=../../build/obj
OBJ_THIS=../../build/obj/parser
BIN_DIR=../../build/bin
BIN_THIS=../../build/bin/parser
SRC_DIR=.



# archivos .c fuente, testdemo.c se eliminará una vez de acabe de desarrollar el parser.
SRC = ast.c parser.c errors.c heredoc.c tokenizer.c redir.c parser_utils.c scanner.c
TEST = test_ast
DEMO_MAIN = testdemo

# archivos .a en $LIB_DIR sin prefijo lib ni extensión .a
# No libs

# LIB_FLAGS = $(addprefix -l, $(LIB))

OBJ=$(SRC:%.c=$(OBJ_THIS)/%.o)
BIN=$(BIN_THIS)/$(TARGET)

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

# Compilar (*.c -> *.o) sin demo
$(OBJ_THIS)/%.o : $(SRC_DIR)/%.c | $(OBJ_THIS)
	@echo "$(PURPLE)--------------------------------------------------------------------------"
	@echo "	Compiling: $(CYAN)$< $(PURPLE) -> $(RED) $@ $(WHITE)"
	$(CC) $(CCFLAGS) -c $< -o $@

# testear submodulo
demo: $(OBJ) $(BIN)

# Vincular (*.o -> binary)
$(BIN) : $(OBJ) | $(BIN_THIS)
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/demo/$(DEMO_MAIN).c -o $(OBJ_THIS)/$(DEMO_MAIN).o
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/test/$(TEST).c -o $(OBJ_THIS)/$(TEST).o
	@echo "$(CYAN)--------------------------------------------------------------------------"
	@echo "	Linking: $(RED)$< $(CYAN)-> $(GREEN)$@$(WHITE)"
	$(CC) $(CCFLAGS) -o $@ $(OBJ) $(OBJ_DIR)/parser/$(DEMO_MAIN).o $(OBJ_DIR)/parser/$(TEST).o
	@echo " "

# Crear carpetas
$(BIN_DIR):
	mkdir -p $@

$(BIN_THIS): | $(BIN_DIR)
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

$(OBJ_THIS): | $(OBJ_DIR)
	mkdir -p $@

clean:
	@echo "$(RED)Removing .o files$(WHITE)"
	@rm -rf $(OBJ_THIS)

debug: all
	$(DBG) ./$(BIN)

.PHONY: all clean debug demo

