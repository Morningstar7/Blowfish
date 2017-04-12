Blowfish: blowfish.h blowfish.c main.c
	gcc -I. blowfish.c main.c -g -o Blowfish -lpthread -Wall 

clean:
		rm -rf *.o Blowfish
