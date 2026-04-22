SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

EXEC_NAMES := array_stats increment barrier polynomial transactions

EXE := $(addprefix $(BIN_DIR)/, $(EXEC_NAMES))
ALL_OBJ := $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(EXEC_NAMES)))

BARRIER_DEPS := $(OBJ_DIR)/barrier-funcs.o
POLYNOMIAL_DEPS := $(OBJ_DIR)/polynomial-funcs.o
TRANSACTIONS_DEPS := $(OBJ_DIR)/transactions-funcs.o


CFLAGS := -pthread -Wall -Iinclude -MMD -MP $(EXTRA_CFLAGS)
LDFLAGS := -pthread



.PHONY: all clean $(EXEC_NAMES)

all: $(EXE)

$(EXEC_NAMES): %: $(BIN_DIR)/%

$(BIN_DIR)/barrier: $(BARRIER_DEPS)

$(BIN_DIR)/polynomial: $(POLYNOMIAL_DEPS)

$(BIN_DIR)/transactions: $(TRANSACTIONS_DEPS)

$(BIN_DIR)/%: $(OBJ_DIR)/%.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ_DIR)/*.d
