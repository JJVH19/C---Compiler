compile: driver.c parser.o
	gcc -Wall -Werror -std=c11 -g -o compile driver.c parser.o
parser.o: parser.c semantics.o ast.o
	gcc -Wall -Werror -std=c11 -g -c parser.c -o parser.o
semantics.o: semantics.c semantics.h parser.h scanner.o
	gcc -Wall -Werror -std=c11 -g -c semantics.c -o semantics.o
ast.o: ast-print.c ast.h
	gcc -Wall -Werror -std=c11 -g -c ast-print.c -o ast.o
scanner.o: scanner.c scanner.h
	gcc -Wall -Werror -std=c11 -g -c scanner.c -o scanner.o
clean:
	rm -f *.o
	rm -f compile