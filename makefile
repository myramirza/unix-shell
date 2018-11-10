myshell: myshell.c
	gcc -Wall myshell.c -o myshell

clean:
	rm lltest $(objects)
