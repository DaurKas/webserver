all: bin bin/server bin/client resources
	
bin:
	mkdir bin

bin/server: src/server.c
	gcc src/server.c -o bin/server -Wall -Werror -lm -fsanitize=undefined -fsanitize=address

bin/client: src/client.c
	gcc src/client.c -o bin/client -Wall -Werror -lm -fsanitize=undefined -fsanitize=address
resources: resources/cgi-sources/hello.c
	gcc resources/cgi-sources/hello.c -o resources/cgi-sources/hello.c -Wall -Werror -lm -fsanitize=undefined -fsanitize=address
clean:
	rm bin/server bin/client
	rmdir bin

