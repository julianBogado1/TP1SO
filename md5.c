// programa que recibe por linea de comando los nombres de los archivos que debe analizar
// deebe iniciar esclavos
// debe distribuir una cantidad mucho menor de los archivos a cada esclavo
//         -->esperar que terminen de obtener el md5, ellos le devuelven el resultado a el, EL LO PONE EN LA SHARE MEMORY
//         -->pasarle mas archivos hasta que se quede sin

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int slave_count = argc / 10;
    char *warn = argv[1];

    for (int i = 0; i < slave_count; i++)
    {
        int childpid = fork();

        if (childpid == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        // if (childpid == 0)
        // { // el hijo leera del pipe
        //     printf("soy el hijo but tdv no soy yo mismoo\n");
        //     int error = execve("./sonCode", execArgs, NULL);
        //     perror("execve");
        //     printf("%d\n", error);
        //     exit(EXIT_FAILURE);
        // }
        // else
        // {
        //     // close(pipefd[0]);

        //     // escribe stdin al pipe
        //     // por lo que se podria interpretar que el pipe es el nuevo stdout

        //     close(pipefd[1]); /* Reader will see EOF */

        //     //OBS: el hijo se completa y queda en modo zombie hasta que
        //     //el programa del padre llega al wait
        //     wait(NULL);       // espera hasta que algun hijo termine
        //     exit(EXIT_SUCCESS);
        // }
    }
}

// fd --> 3
