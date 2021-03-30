all: run

compile: main.c ./pkg/parser/parser.h ./pkg/strings/strings.h
	gcc -g3 -o ./bin/main main.c

run: compile
	./bin/main

test: test.c ./pkg/parser/parser.h
	gcc test.c -o test
	./test
