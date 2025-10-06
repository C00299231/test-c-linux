#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

// pipe code from https://www.geeksforgeeks.org/c/pipe-system-call/
// file reading code from www.w3schools.com/c/c_files_read.php
// curl DL code fromstackoverflow.com/questions/11471690/curl_h_no_such_file_or_directory

// amount of parallel downloaders
#define max_pids 4
#define buffer_size 100

char readURL(int);

int main()
{
    int parentPid = getpid();
    pid_t pids[max_pids];

    char inBuffer[buffer_size];
    
    // pipe stuff
    int pipefd[2];

    pipe(pipefd);

    printf("Parent PID: %d\n", getpid());

    for(int idx = 0; idx < max_pids; idx++) // create each child
    {
        pid_t pid = fork();

        if(pid < 0) // failed child process
        {
            printf("child %d failed to fork...\n", idx);
        }
        
        if(pid == 0) // -------------------------------- CHILD PROCESS
        {
            close(pipefd[1]); // close write end of pipe
            sleep(idx+1);
            printf("DOWNLOADER!\n");
            
            while(1)
            {
                int bytesRead = read(pipefd[0], inBuffer, buffer_size-1);
                printf("%s\n", inBuffer);

                if bytesRead == 0
                {
                    break;
                }
            }

            printf("Child %d (PID %d) exiting...\n", idx, getpid());
            exit(EXIT_SUCCESS);
            
        }
        else // ---------------------------------------- PARENT PROCESS
        {
            close(pipefd[0]); // close read end of pipe
            pids[idx] = pid;
        }
    }

    printf("PARENT!\n\n");

    for(int i = 0; i < max_pids; i++)
    {
        char* url = "test Thingy!!!!";

        if(*url == '0') // no more valid URLs
        {
            break;
        }

        write(pipefd[1], url, buffer_size); // write to buffer for each child!
    }

    close(pipefd[1]); // close write end of pipe
    

    for(int i = 0; i < max_pids; i++) // wait for each process to finish
    {
        int status;
        waitpid(pids[i], &status, 0); // halt parent process until child process terminates (PID, ref to status, no options)
        printf("%d (child %d) exited with status %d\n\n", pids[i], i, WEXITSTATUS(status));
    }

    return 0;
}

char readURL(int nextIndex)
{
    // read from input file

    // get line at input

    // if prefix, ignore and go to next

    // if end of file, return NULL

    return '0';
}