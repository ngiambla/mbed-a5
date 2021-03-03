all: part5 part5.lineclear

part5:
	gcc -Wall part5.c -o part5.exe -I..
	./LoadModules.sh

part5.lineclear:
	gcc -Wall part5.lineclear.c -o part5.lineclear.exe -I..
	./LoadModules.sh

clean:
	rm -f part5.exe part5.lineclear.exe

.PHONY: part5 part5.lineclear clean