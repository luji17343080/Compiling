CC := gcc
PAR := -std=c99 -Iinclude -w
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
	@$(CC) $(PAR) -c -o $(OBJ)/main.o $(SRC)/main.c
	@$(CC) $(PAR) $(OBJ)/main.o -o $(BIN)/parser
	@./$(BIN)/parser $(TEST) $(RESULT)/reslut.ast
clean:
	@rm -rf $(OBJ)
	@rm -rf $(BIN)
	@rm -rf $(RESULT)