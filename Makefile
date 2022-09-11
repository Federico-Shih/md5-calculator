CC=gcc
GCCFLAGS =-Wall -std=c99 -pedantic
GCCLIBS = -lrt -lpthread
FILES = $(wildcard lib/*.c)
OBJECT_FILES = $(FILES:.c=.o)
MAIN_FILES = app view child
DEPS = $(wildcard *.h) $(wildcard lib/*.h)

debug: GCCFLAGS+=-g
debug: all

all: $(MAIN_FILES) $(OBJECT_FILES)

%.o: %.c $(FILES) $(DEPS)
	@$(CC) $(GCCFLAGS) -c $< -o $@

$(MAIN_FILES): %: %.o $(OBJECT_FILES)
	@$(CC) $(GCCFLAGS) $< $(OBJECT_FILES) -o $@ $(GCCLIBS)

.PHONY: clean all
clean:
	@rm -rf $(MAIN_FILES) *.txt $(OBJECT_FILES) *.o