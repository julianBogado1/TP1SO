gcc -c -Wall -g -m32 ./$1.c -o $1.o       #compilar con -g para el gdb
gcc -m32 ./$1.o -o $1