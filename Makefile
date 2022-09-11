CC=gcc
GCCFLAGS = -g -Wall -std=c99 #sacar el -g !!!
GCCLIBS = -lrt -lpthread
FILES = lib/*.c
MAIN_FILES = app view child


all: $(MAIN_FILES)

$(MAIN_FILES): %: %.c
	@$(CC) $(GCCFLAGS) $(FILES) -o $@ $< $(GCCLIBS)

.PHONY: clean
clean:
	@rm -rf $(MAIN_FILES) *.txt


# Posible nuevo makefile
# CC=gcc
# GCCFLAGS = -g -Wall -std=c99 #sacar el -g !!!
# GCCLIBS = -lrt -lpthread
# FILES = $(wildcard lib/*.c)
# OBJECT_FILES = $(FILES:.c=.o)
# MAIN_FILES = app view child
# DEPS = $(wildcard *.h) $(wildcard lib/*.h)

# all: $(MAIN_FILES) $(OBJECT_FILES)

# %.o: %.c $(FILES) $(DEPS)
# 	$(CC) $(GCCFLAGS) -c $< -o $@

# $(MAIN_FILES): %: %.o $(OBJECT_FILES)
# 	$(CC) $(GCCFLAGS) $< -o $@ $(GCCLIBS)

# .PHONY: clean all
# clean:
# 	@rm -rf $(MAIN_FILES) *.txt