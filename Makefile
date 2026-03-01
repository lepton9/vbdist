SRC := ./src
BIN := ./bin
LIB := ./lib
INCLUDE := ./include
BUILD := $(BIN)/build
OBJS := ./objs
INC := -I$(INCLUDE) -I$(LIB)

# Build config: `make CONFIG=debug` or `make CONFIG=release`
CONFIG ?= release

OPTIMIZE_RELEASE := -O3
OPTIMIZE_DEBUG := -Og

DEBUG_FLAGS := -g3 -fno-omit-frame-pointer
RELEASE_FLAGS := -DNDEBUG

OPTIMIZE = $(if $(filter debug,$(CONFIG)),$(OPTIMIZE_DEBUG),$(OPTIMIZE_RELEASE))
CFG_FLAGS = $(if $(filter debug,$(CONFIG)),$(DEBUG_FLAGS),$(RELEASE_FLAGS))
FLAGS = $(OPTIMIZE) $(CFG_FLAGS) -Wextra -Wall
LINK := -L$(LIB) -lm

PLATFORM := $(shell uname)
ifeq  ($(PLATFORM),Linux)
	LINK := $(LINK) -lncurses -ltinfo
endif

CC := gcc
MAIN := vbdist

TESTS := ./tests
TEST_TARGETS := 

OBJ := player team tui tuiswap tuidb tuiskills combo args sql dlist log file utils render skill listarea tuicombo config position generate tuipositions playeredit
OBJECT_FILES := $(addprefix $(OBJS)/,$(addsuffix .o,$(OBJ)))

$(MAIN): $(LIB)/sqlite3.o $(OBJECT_FILES) | $(BIN)
	$(CC) $(FLAGS) $(INC) $^ $(SRC)/$@.c -o $(BIN)/$@ $(LINK)

$(OBJS)/%.o: $(SRC)/%.c | $(OBJS)
	$(CC) $(FLAGS) -c $(INC) $< -o $@

$(LIB)/sqlite3.o: $(LIB)/sqlite3.c
	$(CC) $(FLAGS) -c $(INC) $< -o $@

build: CONFIG := release
build: $(LIB)/sqlite3.o $(OBJECT_FILES) $(OBJS)/$(MAIN).o | $(BUILD)
	$(CC) $(FLAGS) -static $^ -o $(BUILD)/$(MAIN) $(LINK)

debug: CONFIG := debug
debug: $(LIB)/sqlite3.o | $(BIN)
	$(CC) $(FLAGS) $(INC) $^ $(SRC)/*.c -o $(BIN)/$@ $(LINK)
	gdb -tui $(BIN)/debug

MKDIR_P := mkdir -p

$(OBJS):
	$(MKDIR_P) $(OBJS)

$(BIN):
	$(MKDIR_P) $(BIN)

$(BUILD): $(BIN)
	$(MKDIR_P) $(BUILD)


SQLITE_REGEX := [0-9]+/sqlite-amalgamation-[0-9]+\.zip
SQLITE_BASE_URL := https://sqlite.org
SQLITE_SHARED = libsqlite3.so
SQLITE_DOWNLOAD_PATH = $(shell curl -fsSL $(SQLITE_BASE_URL)/download.html \
					 | grep -Eo '$(SQLITE_REGEX)' \
					 | head -n 1)

ifeq ($(OS),Windows_NT)
	SQLITE_SHARED = sqlite3.dll
else
	SQLITE_SHARED = libsqlite3.so
endif

SQLITE_URL = $(SQLITE_BASE_URL)/$(SQLITE_DOWNLOAD_PATH)
SQLITE_ZIP = $(notdir $(SQLITE_DOWNLOAD_PATH))

dep: CONFIG := release
dep:
	@echo "Fetching URL:" $(SQLITE_URL)
	curl -fLo $(SQLITE_ZIP) "$(SQLITE_URL)"
	unzip -j $(SQLITE_ZIP) -d $(LIB)
	rm -f $(SQLITE_ZIP)
	cd $(LIB) && \
	$(CC) -o sqlite3.o -c -fPIC sqlite3.c $(FLAGS) && \
	$(CC) -shared -o $(SQLITE_SHARED) sqlite3.o $(LINK)

#Testing
test: all_tests
	@for target in $(TEST_TARGETS); do \
		$(TESTS)/bin/$$target; \
	done

all_tests: $(addprefix $(TESTS)/bin/, $(TEST_TARGETS))

$(TESTS)/bin/%_test: ../testLibC/utestC.c $(TESTS)/%_test.c $(OBJ)
	$(CC) $(INC) $^ -g -o $@ $(LINK)

clean:
	rm -rf $(OBJS)/*.o $(BIN)/* $(TESTS)/bin/*

cleanall:
	rm -rf $(BIN) $(OBJS) $(LIB) $(TESTS)

fresh: clean $(MAIN)

valgrind:
	valgrind --leak-check=full -s $(BIN)/$(MAIN) $(ARGS)

run:
	$(BIN)/$(MAIN) $(ARGS)
