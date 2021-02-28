#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    // initialize random
    srand(time(NULL));

    int m = 1;
    // check for arguments
    if (argc < 2)
    {
        printf("    ! Not enough arguments are given! (expected 1, given %d)\n", argc - 1);
        printf("      continuing with default values b=1 m=1\n");
        exit(-1);
    }
    else 
    {
        m = atoi(argv[1]); // expected positive
    }

    for (size_t i = 0; i < m; i++)
    {
        char scan = 0;
        scanf("%c", &scan);
        printf("%c", scan);
    }

    printf("\n");
    
    return 0;
}
