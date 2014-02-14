all: main lib.so 
 
main: main.c 
	gcc -rdynamic -o main main.c -ldl 
 
lib.so: lib.c 
	gcc -shared -fPIC -o lib.so lib.c 
 
clean: 
	rm -f main lib.so