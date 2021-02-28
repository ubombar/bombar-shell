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
        int gen = rand() % 3;
        if (gen == 0) // generate number char
        {
            int r = (rand() % (57 - 48 + 1)) + 48;
            printf("%c", (char) r);
        }
        else if (gen == 1)  // generate uppercase letter
        {
            int r = (rand() % (65 - 90 + 1)) + 65;
            printf("%c", (char) r);
        }
        else if (gen == 2)  // generate lowercase letter
        {
            int r = (rand() % (97 - 122 + 1)) + 97;
            printf("%c", (char) r);
        }
    }
    
    return 0;
}
