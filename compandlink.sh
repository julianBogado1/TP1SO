#TUTORIAL PARA USAR ESTA VAINA
#
#en la terminal escribir:
#
#   ./compandlink.sh nombrearchivo_sin_extension
#
#
#Recordar no poner la extension al final, porque ya se tiene en cuenta en el script



#nasm ‐f elf32 $2.asm
gcc -c -g -m32 ./$1.c -o $1.o       #compilar con -g para el gdb
gcc -m32 ./$1.o -o $1
