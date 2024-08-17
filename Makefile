SRC := ./src
BIN := ./bin
OBJS := ./objs
INC := -I ./include
FLAGS := -c $(INC)
LINK := 
CC := gcc
MAIN := vbdist

TESTS := ./tests
TEST_TARGETS := 

OBJ := player team 

$(MAIN): $(addprefix $(OBJS)/,$(addsuffix .o,$(OBJ))) | $(BIN)
	$(CC) $^ $(SRC)/$@.c -o $(BIN)/$@ $(LINK)

$(OBJS)/%.o: $(SRC)/%.c | $(OBJS)
	$(CC) $(FLAGS) $< -o $@

$(OBJS):
	mkdir $(OBJS)

$(BIN):
	mkdir $(BIN)

debug:
	$(CC) $(INC) $(SRC)/*.c -pthread -g -o $(BIN)/db $(LINK)
	gdb -tui $(BIN)/db


#Testing
test: all_tests
	@for target in $(TEST_TARGETS); do \
		$(TESTS)/bin/$$target; \
	done

all_tests: $(addprefix $(TESTS)/bin/, $(TEST_TARGETS))

$(TESTS)/bin/%_test: ../testLibC/utestC.c $(TESTS)/%_test.c $(OBJ)
	$(CC) $(INC) $^ $(LINK) -g -o $@

build: 
	gcc $(SRC)/* -static -o $(BIN)/$(MAIN)Static

clean:
	rm -rf $(OBJS)/*.o $(BIN)/*
	rm -rf $(TESTS)/bin/*

run:
	$(BIN)/$(MAIN) players.txt 1
