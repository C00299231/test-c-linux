#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

// first one will be reader, the rest downloaders
#define max_pids 4

int main()
{
    int parentPid = getpid();
    pid_t pids[max_pids];

    printf("Parent PID: %d\n", getpid());

    for(int idx = 0; idx < max_pids; idx++) // create each child
    {
        pid_t pid = fork();

        if(pid < 0) // failed child process
        {
            printf("child %d failed to fork...\n", idx);
        }
        
        if(pid == 0) // uninitialized value: means is child process
        {
            if (idx == 0)
            {
                // child is reader! do reading stuff
                printf("READER!\n");
            }
            else
            {
                // child is downloader! do downloading stuff
                printf("DOWNLOADER!\n");
            }

            
            exit(EXIT_SUCCESS);
        }
        else // parent process
        {
            pids[idx] = pid;
        }
    }

    for(int i = 0; i < max_pids; i++) // wait for each process to finish
    {
        int status;
        waitpid(pids[i], &status, 0); // halt parent process until child process terminates (PID, ref to status, no options)
        printf("%d (child %d) exited with status %d\n", pids[i], i, WEXITSTATUS(status));
    }

    return 0;
}
