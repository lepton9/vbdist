SRC := ./src
BIN := ./bin
BUILD := $(BIN)/build
OBJS := ./objs
INC := -I ./include
LIB := -L ./lib
FLAGS := -c $(INC)
LINK := -lsqlite3 -lm

PLATFORM := $(shell uname)
ifeq  ($(PLATFORM),Linux)
	LINK := $(LINK) -lncurses -ltinfo
endif

CC := gcc
MAIN := vbdist

TESTS := ./tests
TEST_TARGETS := 

OBJ := player team tuiSwitch combo mark args sql
OBJECT_FILES := $(addprefix $(OBJS)/,$(addsuffix .o,$(OBJ)))

$(MAIN): $(OBJECT_FILES) | $(BIN)
	$(CC) $^ $(SRC)/$@.c -o $(BIN)/$@ $(LIB) $(LINK)

$(OBJS)/%.o: $(SRC)/%.c | $(OBJS)
	$(CC) $(FLAGS) $< -o $@

$(OBJS):
	mkdir $(OBJS)

$(BIN):
	mkdir $(BIN)

$(BUILD): $(BIN)
	mkdir $(BUILD)

build: $(OBJECT_FILES) $(OBJS)/$(MAIN).o | $(BUILD)
	$(CC) -static $^ -o $(BUILD)/$(MAIN) $(LIB) $(LINK)

debug:
	$(CC) $(INC) $(SRC)/*.c -pthread -g -o $(BIN)/db $(LINK)
	gdb -tui $(BIN)/db

SQLITE_VER := 3470200
dep:
	curl -O https://www.sqlite.org/2024/sqlite-amalgamation-$(SQLITE_VER).zip
	unzip -j sqlite-amalgamation-$(SQLITE_VER).zip -d lib
	rm -f sqlite-amalgamation-$(SQLITE_VER).zip
	cd lib && \
	gcc -o sqlite3.o -c -fPIC sqlite3.c && \
	gcc -shared -o sqlite3.so sqlite3.o -lm && \
	ar rcs libsqlite3.a sqlite3.o


#Testing
test: all_tests
	@for target in $(TEST_TARGETS); do \
		$(TESTS)/bin/$$target; \
	done

all_tests: $(addprefix $(TESTS)/bin/, $(TEST_TARGETS))

$(TESTS)/bin/%_test: ../testLibC/utestC.c $(TESTS)/%_test.c $(OBJ)
	$(CC) $(INC) $^ -g -o $@ $(LINK)

clean:
	rm -rf $(OBJS)/*.o $(BIN)/*
	rm -rf $(TESTS)/bin/*

run:
	$(BIN)/$(MAIN)
