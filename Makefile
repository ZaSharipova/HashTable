CC      = g++
CFLAGS  = -Wall -Wextra -O3
OBJ_DIR = obj
BIN_DIR = bin
GEN_DIR = data
USE_DIR = use

COMMON_SRCS = main.cpp HashFunctions.cpp CommonFunctions.cpp ChainTable0.cpp
COMMON_OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(COMMON_SRCS)) MyStrlen.o

.PHONY: all GenerateString GenerateQueries result
all: generate result

generate: | $(BIN_DIR) $(GEN_DIR)
	@$(CC) $(CFLAGS) GenerateString.cpp CommonFunctions.cpp -o $(BIN_DIR)/gen_tmp -lm
	@$(BIN_DIR)/gen_tmp > $(GEN_DIR)/tests_string.txt
	@$(CC) $(CFLAGS) GenerateQueries.cpp CommonFunctions.cpp -o $(BIN_DIR)/gen_tmp -lm
	@$(BIN_DIR)/gen_tmp > $(GEN_DIR)/tests_queries.txt

result: $(COMMON_OBJS) | $(BIN_DIR)
	@$(CC) $(CFLAGS) -msse4.2  -mavx2 $^ -o $(BIN_DIR)/result -lm
	@./bin/result

SHELL = /bin/bash

COUNTER_FILE = .callgrind_counter

valg: $(BIN_DIR)/result
	@NAME=$(if $(NAME),$(NAME),run); 																	\																		\
	echo "=== Запуск callgrind с именем: $${NAME} ==="; 												\
	valgrind --tool=callgrind --callgrind-out-file=use/callgrind_raw_$${NAME}.txt ./$(BIN_DIR)/result; 	\
	callgrind_annotate --auto=yes use/callgrind_raw_$${NAME}.txt > use/callgrind_$${NAME}.txt; 			\
	rm use/callgrind_raw_$${NAME}.txt; 																	\
	echo "=== Расшифровка в use/callgrind_$${NAME}.txt ==="

MyStrlen.o: MyStrlen.s
	@nasm -f elf64 $< -o $@ 

$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR) $(BIN_DIR) $(GEN_DIR) $(USE_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)