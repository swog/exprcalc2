
CC := g++ -D DEBUG
LD := ld

SRC_FILES := $(shell find src -name *.cpp)
BIN_FILES := $(patsubst src/%.cpp, bin/%.o, $(SRC_FILES))

$(BIN_FILES): $(SRC_FILES)
	mkdir -p bin && \
	$(CC) -c -I./include $(patsubst bin/%.o, src/%.cpp, $@) -o $@

.PHONY: clean run all clearsc rebuild

clean: clearsc 
	rm -rf bin && \
	rm -f exprcalc

exprcalc: $(BIN_FILES)
	$(CC) $(BIN_FILES) -o $@

all: exprcalc

clearsc: 
	clear

rebuild: clean all
