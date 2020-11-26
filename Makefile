all: bin bin/server bin/client resources/cgi-source/hello
	
bin:
	mkdir bin

bin/server: src/server.c
	gcc src/server.c -o bin/server -Wall -Werror -lm -fsanitize=undefined -fsanitize=address

bin/client: src/client.c
	gcc src/client.c -o bin/client -Wall -Werror -lm -fsanitize=undefined -fsanitize=address

resources/cgi-source/hello: resources/cgi-source/hello.c
	mkdir resources/cgi-bin
	gcc resources/cgi-source/hello.c -o resources/cgi-bin/hello -Wall -Werror -lm -fsanitize=undefined -fsanitize=address

clean:
	rm bin/server bin/client
	rmdir bin
	rm resources/cgi-bin/hello
	rmdir rsources/cg-bin

