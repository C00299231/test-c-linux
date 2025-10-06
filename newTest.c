#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

// pipe code from https://www.geeksforgeeks.org/c/pipe-system-call/
// file reading code from www.w3schools.com/c/c_files_read.php
// curl DL code fromstackoverflow.com/questions/11471690/curl_h_no_such_file_or_directory

// PROGRAM STRUCTURE
// PARENT:
//      CREATION LOOP:
//          create pipes
//          fork child

//      PROCESSING LOOP:
//          read from up pipe of children to get next ready child
//          if ready child:
//              read next URL from file
//              write URL to down pipe of child
//              if no more URLs:
//                  end program
//          if no ready child:
//              wait for child to finish

// CHILD:
//      LOOP:
//          read from down pipe for URL
//          if URL:
//              download URL
//              write to up pipe result
//          if no URL:
//              wait
//          if pipe closed:
//              exit

// amount of parallel downloaders
#define max_pids 3
#define buffer_size 100

char readURL(int);
int downloadToFile(const char *url, char *filename, int index);

int main()
{
    int parentPid = getpid();
    pid_t pids[max_pids];

    char inBuffer[buffer_size];
    
    // pipe stuff
    int downPipes[max_pids][2];
    int upPipes[max_pids][2];
    

    printf("Parent PID: %d\n", getpid());

    //---------------------------------------------------CREATION LOOP:
    for(int idx = 0; idx < max_pids; idx++) // create each child
    {

        // create pipes
        pipe(downPipes[idx]);
        pipe(upPipes[idx]);

        pid_t pid = fork();

        if(pid < 0) // failed child process
        {
            printf("child %d failed to fork...\n", idx);
        }
        
        if(pid == 0) // CHILD PROCESS
        {
            close(downPipes[idx][1]); // close write end of pipe
            sleep(idx+1);
            printf("DOWNLOADER!\n");
            
            while(1) // child keeps going until write end of pipe is closed
            {
                int bytesRead = read(downPipes[idx][0], inBuffer, buffer_size-1); // put pipe data into inBuffer, status into bytesRead
                
                printf("%s\n", inBuffer);

                if(bytesRead == 0) // if pipe is closed
                {
                    break;
                }
            }

            printf("Child %d (PID %d) exiting...\n", idx, getpid());
            exit(EXIT_SUCCESS);
            // end of child process
        }
        else // PARENT PROCESS
        {
            printf("PARENT!\n\n");
            close(downPipes[idx][0]); // close read end of pipe
            pids[idx] = pid;
        }
    }

    //---------------------------------------------------PROCESSING LOOP:
    while(1)
    {
        int readyChild = -1;
        // check up pipes for ready children
        for(int i = 0; i < max_pids; i++)
        {
            // read from up pipe of children to get next ready child
            int bytesRead = read(upPipes[i][0], inBuffer, buffer_size-1); // put pipe data into inBuffer, status into bytesRead
            if(bytesRead > 0) // if ready child
            {
                readyChild = i;
                break;
            }
        }
    
        if(readyChild != -1)
        {
            // child is ready
            char* url = "test URL";

            if(*url == '0') // no more valid URLs
            {
                break; // end program
            }

            write(downPipes[readyChild][1], url, buffer_size); // write to buffer for ready child
        }
        else
        {
            // do nothing...
        }
    }

    // program is done, clean up and exit

    for(int i = 0; i < max_pids; i++) // close all pipes, children will see closed pipes and exit
    {
        close(downPipes[i][1]);
    }

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

int downloadToFile(const char *url, char *filename, int index)
{
    // init curl and file
    CURL *curl = curl_easy_init();
    FILE *fp = fopen(filename, "wb");

    // errors
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        return 0; // failure
    }
    if (!fp)
    {
        printf("Failed to open file");
        curl_easy_cleanup(curl);
        return 0;
    }

    // set curl options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); // find reference for writedata

    CURLcode res = curl_easy_perform(curl);
    fclose(fp);

    // more errors
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return 0;
    }

    curl_easy_cleanup(curl);
    return 1; // success
}