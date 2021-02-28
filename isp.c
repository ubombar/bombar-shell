#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define CMD_BUFFER 256
#define NUM_BUFFERS 32
#define TOKEN_BUFFER 256

#define READ 0
#define WRITE 1

typedef struct
{
    char* name;
    int len;
} token;

void fill(char* _str, char c, int len) 
{
    for (size_t i = 0; i < len; i++)
        _str[i] = c;
}

void replace(char *str, char orig, char rep) 
{
    char *current = str;
    while((current = strchr(current, orig)) != NULL) 
    {
        *current++ = rep;
    }
}

int count(char *str, char c) 
{
    int n = 0;
    char *current = str;
    while((current = strchr(current, c)) != NULL) 
    {
        n++;
    }
    return n;
}

void trim(char* _str, char* _new, char _c)
{
    _new[0] = 0;

    if (_str == NULL || strlen(_str) == 0) return;

    int i = 0;
    int j = 0;
    char prevoius = _c;
    char current;

    while ((current = _str[i]) != 0)
    {
        if (current != _c || prevoius != _c)
        {
            _new[j] = current;
            j += 1;
        }

        i += 1;
        prevoius = current;
    }
    if (j >= 1 && _new[j - 1] == _c)
    {
        _new[j - 1] = 0;
    }
}

int generate_tokens(token* token_buffer, char* _str) 
{
    if (strlen(_str) == 0) return 0;

    int n = 0;

    char* split = strtok(_str, " ");

    token_buffer[n] = (token) {
        split, 
        strlen(split)
    };

    while ((split = strtok(NULL, " ")) != NULL)
    {
        n += 1;
        token_buffer[n] = (token) {
            split, 
            strlen(split)
        };
    }
    

    return n + 1;
}

int main(int argc, char const *argv[])
{
    // debug mode print insigts
    int debug_mode = 1; // set this 0 to disable debug
    
    if (debug_mode)
    {
        char* l1 = "     ____                  _                   _____ _          _ _ ";
        char* l2 = "    |  _ \\                | |                 / ____| |        | | |";
        char* l3 = "    | |_) | ___  _ __ ___ | |__   __ _ _ __  | (___ | |__   ___| | |";
        char* l4 = "    |  _ < / _ \\| '_ ` _ \\| '_ \\ / _` | '__|  \\___ \\| '_ \\ / _ \\ | |";
        char* l5 = "    | |_) | (_) | | | | | | |_) | (_| | |     ____) | | | |  __/ | |";
        char* l6 = "    |____/ \\___/|_| |_| |_|_.__/ \\__,_|_|    |_____/|_| |_|\\___|_|_|";
                                                                 
        printf("%s\n%s\n%s\n%s\n%s\n%s\n\n", l1, l2, l3, l4, l5, l6);
        printf("!! Running on debug mode on, this will display the parsed input and time !!\n\n");
    }

    // define the buffers in stack
    char in_buffer[CMD_BUFFER];
    char cmd_buffer[CMD_BUFFER];
    token token_buffer[TOKEN_BUFFER];

    int no_bytes = 64; // expected positive
    int mode = 2; // expected 1 (normal) or 2 (tapped)

    // check for arguments
    if (argc < 3)
    {
        printf("%d", argc);
        printf("    ! Not enough arguments are given! (expected 2, given %d)\n", argc - 1);
        printf("      continuing with default values b=%d m=%d\n\n", no_bytes, mode);
    }
    else 
    {
        int _no_bytes = atoi(argv[1]); // expected positive
        int _mode = atoi(argv[2]); // expected 1 (normal) or 2 (tapped)

        if (_mode != 1 || _mode != 2)
        {
            printf("    ! Mode can only be 1 (normal mode) or 2 (tapped mode) (continuing m=%d)\n\n", mode);
        }
        else 
        {
            mode = _mode;
        }
        if (_no_bytes <= 0 || _no_bytes > 4096)
        {
            printf("    ! No of bytes can be positive integers and maximum 4096 (continuing b=%d)\n\n", no_bytes);
        }
        else 
        {
            no_bytes = _no_bytes;
        }
    }

    while (1)
    {
        // clean the buffers
        fill(in_buffer, 0, CMD_BUFFER);
        fill(cmd_buffer, 0, CMD_BUFFER);
        fill((char*) token_buffer, 0, CMD_BUFFER);

        // write the prompt
        printf("bombar-shell$ ");

        // get the input and remove the new line and then trim
        fgets(in_buffer, CMD_BUFFER - 1, stdin);
        replace(in_buffer, '\n', 0);
        trim(in_buffer, cmd_buffer, ' ');

        if (debug_mode) 
            printf(">   '%s'\n", cmd_buffer); // Comment this out for debug

        // generate tokens (alters the cmd_buffer)
        int no_tokens = generate_tokens(token_buffer, cmd_buffer);

        // if the command is piped or not
        int pipe_symbol = -1;

        for (size_t i = 0; i < no_tokens; i++)
            if (strcmp(token_buffer[i].name, "|") == 0)
                pipe_symbol = i;


        // if single command is given
        if (pipe_symbol == -1) 
        {
            // create arguments
            char* args[no_tokens + 1];
            for (size_t i = 0; i < no_tokens; i++)
            {
                args[i] = token_buffer[i].name;
            }

            args[no_tokens] = 0;

            pid_t p = fork();
            
            // child's perspective
            if (p == 0)
            {
                int ret = execvp(token_buffer[0].name, args);
                if (ret < 0 && debug_mode)
                {
                    printf("    ! Execution failed (exited with status code %d)\n", ret);
                }
                exit(0);
            }
            // parent's perspective
            if (p != 0) 
            {
                wait(NULL);
            }
        }

        // if double commands are given and in normal mode
        if (pipe_symbol != -1 && mode == 1)
        {
            int fd[2];
            // create arguments
            char* arg1[pipe_symbol + 1];
            char* arg2[no_tokens - pipe_symbol];

            for (size_t i = 0; i < no_tokens; i++)
            {
                if (i < pipe_symbol)
                {
                    arg1[i] = token_buffer[i].name;
                }
                else if (i == pipe_symbol)
                {
                    arg1[i] = 0;
                }
                else if (i > pipe_symbol)
                {
                    arg2[i - pipe_symbol - 1] = token_buffer[i].name;
                }
            }

            arg2[no_tokens - pipe_symbol - 1] = 0;

            pipe(fd);
            pid_t p1 = fork();
            
            // Producer's perspective
            if (p1 == 0)
            {
                dup2(fd[WRITE], STDOUT_FILENO);
                close(fd[READ]);

                int ret = execvp(token_buffer[0].name, arg1);
                if (ret < 0 && debug_mode)
                {
                    printf("    ! Child 1 execution failed (exited with status code %d)\n", ret);
                }
                
                close(fd[WRITE]);
                exit(0);
            }

            pid_t p2 = fork();

            // Consumer's perspective
            if (p2 == 0)
            {
                dup2(fd[READ], STDIN_FILENO);
                close(fd[WRITE]);

                int ret = execvp(token_buffer[pipe_symbol + 1].name, arg2);
                if (ret < 0 && debug_mode)
                {
                    printf("    ! Child 2 execution failed (exited with status code %d)\n", ret);
                }

                close(fd[READ]);
                exit(0);
            }

            // parent's perspective
            if (p2 != 0) 
            {
                int parent_std_inn = dup(STDIN_FILENO);
                int parent_std_out = dup(STDOUT_FILENO);

                close(fd[READ]);
                close(fd[WRITE]);

                wait(NULL);
                wait(NULL);

                dup2(STDIN_FILENO, parent_std_inn);
                dup2(STDOUT_FILENO, parent_std_out);
            }
        }

        // if double commands are given and in tapped mode
        if (pipe_symbol != -1 && mode == 2)
        {
            int fd1[2];
            int fd2[2];

            // create arguments
            char* arg1[pipe_symbol + 1];
            char* arg2[no_tokens - pipe_symbol];

            for (size_t i = 0; i < no_tokens; i++)
            {
                if (i < pipe_symbol)
                {
                    arg1[i] = token_buffer[i].name;
                }
                else if (i == pipe_symbol)
                {
                    arg1[i] = 0;
                }
                else if (i > pipe_symbol)
                {
                    arg2[i - pipe_symbol - 1] = token_buffer[i].name;
                }
            }

            arg2[no_tokens - pipe_symbol - 1] = 0;

            pipe(fd1);
            pipe(fd2);

            pid_t p1 = fork();
            
            // Producer's perspective
            if (p1 == 0)
            {
                close(fd2[WRITE]);
                close(fd2[READ]);

                close(fd1[READ]);
                dup2(fd1[WRITE], STDOUT_FILENO);
                close(fd1[WRITE]);

                int ret = execvp(token_buffer[0].name, arg1);
                if (ret < 0 && debug_mode)
                {
                    printf("    ! Child 1 execution failed (exited with status code %d)\n", ret);
                }

                exit(0);
            }

            pid_t p2 = fork();

            // Consumer's perspective
            if (p2 == 0)
            {
                close(fd1[WRITE]);
                close(fd1[READ]);

                close(fd2[WRITE]);
                dup2(fd2[READ], STDIN_FILENO);
                close(fd2[READ]);

                int ret = execvp(token_buffer[pipe_symbol + 1].name, arg2);
                if (ret < 0 && debug_mode)
                {
                    printf("    ! Child 2 execution failed (exited with status code %d)\n", ret);
                }

                exit(0);
            }

            // parent's perspective
            if (p2 != 0) 
            {
                close(fd1[WRITE]);
                close(fd2[READ]);

                int parent_std_inn = dup(STDIN_FILENO);
                int parent_std_out = dup(STDOUT_FILENO);

                char parent_buffer[no_bytes];
                int read_bytes;

                // read from producer and write to the consumer
                while ((read_bytes = read(fd1[READ], parent_buffer, no_bytes)) > 0)
                    write(fd2[WRITE], parent_buffer, read_bytes);

                close(fd2[WRITE]);
                close(fd1[READ]);

                wait(NULL);
                wait(NULL);

                dup2(STDIN_FILENO, parent_std_inn);
                dup2(STDOUT_FILENO, parent_std_out);
            }
        }
        
    }
    return 0;
}
