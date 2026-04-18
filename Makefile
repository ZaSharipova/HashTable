CC       = g++
CFLAGS   = -Wall -Wextra -O2 -msse4.2 -mavx2
INCLUDES = -Iinclude -Iinclude/ChainTable

OBJ_DIR  = obj
BIN_DIR  = bin
GEN_DIR  = data
USE_DIR  = callgrind_big

# make result CRC_INTR=1 POOL=1
SRCS  = src/main.cpp            \
        src/CommonFunctions.cpp

ifdef POOL
  	CFLAGS += -D_DPOOL
  	SRCS   += src/ChainTable/ChainTablePool.cpp
else
  	SRCS   += src/ChainTable/ChainTable.cpp
endif

ifdef CRC_INTR
  	CFLAGS += -D_DCRC_INTR
  	SRCS   += src/Crc32/Crc_mm_crc32_u64.cpp
else ifdef CRC_INTR_STRLEN
  	CFLAGS += -D_DCRC_INTR_STRLEN
  	SRCS   += src/Crc32/Crc_mm_crc32_u64_strlen.cpp
else
  	SRCS   += src/HashFunctions.cpp
endif

ifdef SIMD
  	CFLAGS += -D_DSIMD
endif

ifdef PREFETCH
	CFLAGS += -D_DPREFETCH
endif


# ifdef POOL
#   SRCS += src/ChainTable/ChainTablePool.cpp
# else
#   SRCS += src/ChainTable/ChainTable.cpp
# endif

# ifdef CRC_INTR
#   SRCS += src/Crc32/Crc_mm_crc32_u64.cpp
# endif

OBJS = $(patsubst src/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
ifdef CRC_INTR_STRLEN
	OBJS += MyStrlen.o
endif

ifdef SIMD_S
  OBJS += MyStrcmp.o
endif

.PHONY: all generate result valg clean

all: generate result

generate: | $(BIN_DIR) $(GEN_DIR)
	@$(CC) $(CFLAGS) $(INCLUDES) src/Additional/GenerateString.cpp src/CommonFunctions.cpp -o $(BIN_DIR)/gen_str
	@$(BIN_DIR)/gen_str > $(GEN_DIR)/tests_string.txt
	@$(CC) $(CFLAGS) $(INCLUDES) src/Additional/GenerateQueries.cpp src/CommonFunctions.cpp -o $(BIN_DIR)/gen_qry
	@$(BIN_DIR)/gen_qry > $(GEN_DIR)/tests_queries.txt

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