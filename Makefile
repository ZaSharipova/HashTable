CC       = g++
CFLAGS   = -Wall -Wextra -O3 -msse4.2 -mavx2 -g
INCLUDES = -Iinclude -Iinclude/ChainTable

OBJ_DIR  = obj
BIN_DIR  = bin
GEN_DIR  = data
USE_DIR  = callgrind_list

COMMON_SRCS = src/HashFunctions.cpp src/CommonFunctions.cpp src/Test1.cpp
COMMON_OBJS = $(patsubst src/%.cpp, $(OBJ_DIR)/%.o, $(COMMON_SRCS))
TEST1_OBJ = $(OBJ_DIR)/Test1.o

SRCS  = src/main.cpp            \
        src/CommonFunctions.cpp

ifdef POOL
  	CFLAGS += -D_DPOOL
  	SRCS   += src/ChainTable/ChainTablePool.cpp
else ifdef LIST_TABLE
  	CFLAGS += -D_DLIST_TABLE
  	SRCS   += src/ChainTable/ChainTableList.cpp
else
  	SRCS   += src/ChainTable/ChainTable.cpp
endif

ifdef CRC_INTR
  	CFLAGS += -D_DCRC_INTR
	SRCS   += src/HashFunctions.cpp
else ifdef CRC_INTR_STRLEN
  	CFLAGS += -D_DCRC_INTR_STRLEN
  	SRCS   += src/Crc32/Crc_mm_crc32_u64_strlen.cpp
	SRCS   += src/HashFunctions.cpp
else
  	SRCS   += src/HashFunctions.cpp
endif

ifdef SIMD
  	CFLAGS += -D_DSIMD
endif

ifdef CHECK_FIRST_CHAR
	CFLAGS += -D_DCHECK_FIRST_CHAR
endif

OBJS = $(patsubst src/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
ifdef CRC_INTR_STRLEN
	OBJS += MyStrlen.o
endif

ifdef SIMD_S
  OBJS += MyStrcmp.o
endif

.PHONY: all generate result valg clean temp_start temp_stop

all: generate result

generate: | $(BIN_DIR) $(GEN_DIR)
	@$(CC) $(CFLAGS) $(INCLUDES) src/Additional/GenerateString.cpp src/CommonFunctions.cpp -o $(BIN_DIR)/gen_str
	@$(BIN_DIR)/gen_str > $(GEN_DIR)/tests_string.txt
	@$(CC) $(CFLAGS) $(INCLUDES) src/Additional/GenerateQueries.cpp src/CommonFunctions.cpp -o $(BIN_DIR)/gen_qry
	@$(BIN_DIR)/gen_qry > $(GEN_DIR)/tests_queries.txt

test1: $(TEST1_OBJ) $(COMMON_OBJS) | $(BIN_DIR)
	@echo "\n=====Running test1======"
	@$(CC) -Wall -Wextra -O2 $^ -o $(BIN_DIR)/test1 -lm
	@./bin/test1
	@python3 src/Test1Graphic.py

# result:
# 	$(MAKE) clean
# 	$(MAKE) _result $(MAKEFLAGS)
result: $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $(BIN_DIR)/result
	@./$(BIN_DIR)/result

# make valg NAME=opt_v3
valg: $(BIN_DIR)/result | $(USE_DIR)
	@NAME=$${NAME:-run}; 											\
	echo "=== callgrind: $${NAME} ==="; 							\
	valgrind --tool=callgrind 										\
	         --callgrind-out-file=$(USE_DIR)/callgrind_$${NAME}.out \
	         ./$(BIN_DIR)/result; 									\
	callgrind_annotate --auto=yes $(USE_DIR)/callgrind_$${NAME}.out \
	         > $(USE_DIR)/callgrind_$${NAME}.txt; 					\
	echo "=== Текст: $(USE_DIR)/callgrind_$${NAME}.txt ==="; 		\
	kcachegrind $(USE_DIR)/callgrind_$${NAME}.out &

.PHONY: perf
perf: $(BIN_DIR)/result | $(USE_DIR)
	@NAME=$${NAME:-run}; 									\
	echo "=== perf record: $${NAME} ==="; 					\
	perf record -g --call-graph dwarf 						\
		-o $(USE_DIR)/perf_$${NAME}.data 					\
		./$(BIN_DIR)/result; 								\
	perf report -i $(USE_DIR)/perf_$${NAME}.data 			\
		--stdio 											\
		--sort=symbol 										\
		--percent-limit=0.1 								\
		> $(USE_DIR)/perf_$${NAME}.txt;						\
	echo "=== Результат: $(USE_DIR)/perf_$${NAME}.txt ==="


$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

MyStrlen.o: MyStrlen.s
	@nasm -f elf64 $< -o $@ 

MyStrcmp.o: MyStrcmp.s
	@nasm -f elf64 $< -o $@ 

$(OBJ_DIR) $(BIN_DIR) $(GEN_DIR) $(USE_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR) $(USE_DIR)

.PHONY: bench
bench:
	@mkdir -p images
	@rm -f temp.txt
	@( 																			\
		counter=0; 																\
		while true; do 															\
			temp=$$(sensors | grep 'Core 2' | awk '{print $$3}' | tr -d '+°C'); \
			echo "$$counter $$temp"; 											\
			counter=$$((counter + 1)); 											\
			sleep 1; 															\
		done 																	\
	) > temp.txt & 																\
	LOGGER_PID=$$!; 															\
	echo "Starting benchmark..."; 												\
	taskset -c 2 $(MAKE) result $(EXTRA); 										\
	kill $$LOGGER_PID 2>/dev/null; 												\
	gnuplot scripts/plot_temp.gp
	@echo "Done! Plot saved to images/temperature_plot.png"

temp_start:
	@mkdir -p images
	@rm -f temp.txt
	@( 																			\
		counter=0; 																\
		while true; do 															\
			temp=$$(sensors | grep 'Core 2' | awk '{print $$3}' | tr -d '+°C'); \
			echo "$$counter $$temp"; 											\
			counter=$$((counter + 1)); 											\
			sleep 1; 															\
		done 																	\
	) > temp.txt & 																\
	echo $$! > .temp_pid; 														\
	echo "Temperature logging started (PID $$(cat .temp_pid))"

temp_stop:
	@if [ -f .temp_pid ]; then 										\
		kill $$(cat .temp_pid) 2>/dev/null; 						\
		rm -f .temp_pid; 											\
		gnuplot scripts/plot_temp.gp; 								\
		echo "Stopped. Plot saved to images/temperature_plot.png"; 	\
	else 															\
		echo "No logger running"; 									\
	fi