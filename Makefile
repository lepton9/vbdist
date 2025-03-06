SRC := ./src
BIN := ./bin
LIB := ./lib
INCLUDE := ./include
BUILD := $(BIN)/build
OBJS := ./objs
INC := -I$(INCLUDE) -I$(LIB)
FLAGS := -O3 -Wextra -Wall -Wno-unused-parameter
LINK := -L$(LIB)

PLATFORM := $(shell uname)
ifeq  ($(PLATFORM),Linux)
	LINK := $(LINK) -lncurses -ltinfo
endif

CC := gcc
MAIN := vbdist

TESTS := ./tests
TEST_TARGETS := 

OBJ := player team tui tuiswap tuidb tuiskills combo args sql dlist log utils render skill listarea tuicombo
OBJECT_FILES := $(addprefix $(OBJS)/,$(addsuffix .o,$(OBJ)))

$(MAIN): $(OBJECT_FILES) $(LIB)/sqlite3.o | $(BIN)
	$(CC) $(FLAGS) $(INC) $^ $(SRC)/$@.c -o $(BIN)/$@ $(LINK)

$(OBJS)/%.o: $(SRC)/%.c | $(OBJS)
	$(CC) $(FLAGS) -c $(INC) $< -o $@

$(LIB)/sqlite3.o: $(LIB)/sqlite3.c
	$(CC) $(FLAGS) -c $(INC) $< -o $@

$(OBJS):
	mkdir $(OBJS)

$(BIN):
	mkdir $(BIN)

$(BUILD): $(BIN)
	mkdir $(BUILD)

build: $(OBJECT_FILES) $(LIB)/sqlite3.o $(OBJS)/$(MAIN).o | $(BUILD)
	$(CC) -static $^ -o $(BUILD)/$(MAIN) $(LINK)

debug: $(LIB)/sqlite3.o | $(BIN)
	$(CC) $(INC) $^ $(SRC)/*.c -g -o $(BIN)/$@ $(LINK)
	gdb -tui $(BIN)/debug

SQLITE_VER := 3470200
dep:
	curl -O https://www.sqlite.org/2024/sqlite-amalgamation-$(SQLITE_VER).zip
	unzip -j sqlite-amalgamation-$(SQLITE_VER).zip -d lib
	rm -f sqlite-amalgamation-$(SQLITE_VER).zip
	cd lib && \
	gcc -o sqlite3.o -c -fPIC sqlite3.c && \
	gcc -shared -o libsqlite3.so sqlite3.o -lm


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

cleanall: clean
	rm -rf $(BIN) $(OBJS) $(LIB)

run:
	$(BIN)/$(MAIN)
