CC := gcc
CFLAGS := -std=c99 -Iinclude -w
NCLUDE := include
SRC := src
OBJ := obj
BIN := bin
RESULT := result
TEST := test/test.tiny

parser:
	@mkdir -p $(BIN)
	@mkdir -p $(OBJ)
	@mkdir -p $(RESULT)
	@$(CC) $(CFLAGS) $(INCLUDE) -c -o $(OBJ)/main.o $(SRC)/main.c
	@$(CC) $(CFLAGS) $(INCLUDE) $(OBJ)/main.o -o $(BIN)/parser
	@./$(BIN)/parser $(TEST) $(RESULT)/result.ast
clean:
	@rm -rf $(OBJ)
	@rm -rf $(BIN)
	@rm -rf $(RESULT)