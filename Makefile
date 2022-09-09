CC=gcc
GCCFLAGS = -g -Wall -std=c99 #sacar el -g !!!
GCCLIBS = -lrt -lpthread
FILES = shared_memory.c
MAIN_FILES = app view child


all: $(MAIN_FILES)

$(MAIN_FILES): %: %.c
	@$(CC) $(GCCFLAGS) $(FILES) -o $@ $< $(GCCLIBS)

.PHONY: clean
clean:
	@rm -rf $(MAIN_FILES) *.txt