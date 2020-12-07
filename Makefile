CFLAGS := -Wall -Werror -lm -fsanitize=undefined -fsanitize=address
SRC  := ./resources/cgi-source
BIN  := ./resources/cgi-bin
SRCS := $(wildcard $(SRC)/*.c)
EXEC := $(patsubst $(SRC)/%.c,$(BIN)/%,$(SRCS))
.PHONY: all clean
all: bin resources/cgi-bin bin/server bin/client $(EXEC)
	
bin:
	mkdir bin

resources/cgi-bin:
	mkdir resources/cgi-bin

bin/server: src/server.c
	gcc src/server.c -o bin/server -Wall -Werror -lm -fsanitize=undefined -fsanitize=address

bin/client: src/client.c
	gcc src/client.c -o bin/client -Wall -Werror -lm -fsanitize=undefined -fsanitize=address
	
$(BIN)/%: $(SRC)/%.c
	@echo [Compiling]: $@
	gcc $(CFLAGS) $< -o $@

clean:
	rm -r bin
	rm -r resources/cgi-bin

