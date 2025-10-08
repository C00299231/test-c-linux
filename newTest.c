#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <curl/curl.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>

// pipe code from https://www.geeksforgeeks.org/c/pipe-system-call/
// file reading code from https://www.w3schools.com/c/c_files_read.php
// curl DL code from https://www.stackoverflow.com/questions/11471690/curl_h_no_such_file_or_directory
// fgets code from https://en.cppreference.com/w/c/io/fgets
// overwrite file line code from https://stackoverflow.com/questions/33699915/how-to-read-and-edit-specific-line-of-a-text-file-in-c

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
#define max_pids 1
#define buffer_size 128
#define path "filesToDownload.txt"

char* readURL(int);
int downloadToFile(const char *url, char *filename, int index);

int main()
{
    int parentPid = getpid();
    pid_t pids[max_pids];

    char inBuffer[buffer_size];
    char *readyBuffer;
    
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
        fcntl(upPipes[idx][0], F_SETFL, O_NONBLOCK); // make upPipes non-blocking
        fcntl(downPipes[idx][0], F_SETFL, O_NONBLOCK);

        fflush(stdout);
        pid_t pid = fork();

        if(pid < 0) // failed child process
        {
            printf("child %d failed to fork...\n", idx);
        }
        
        if(pid == 0) // CHILD PROCESS
        {
            close(downPipes[idx][1]); // close write end of pipe
            close(upPipes[idx][0]); // close read end of pipe
            printf("DOWNLOADER!\n");

            // first, write ready status
            // child sends same character as URL writer for ready-success, or ready-failure
            // ready no result, send 0

            char send = '0';

            write(upPipes[idx][1], &send, 1);
            
            while(1) // child keeps going until write end of pipe is closed
            {
                int bytesRead = read(downPipes[idx][0], inBuffer, buffer_size-1); // put pipe data into inBuffer, status into bytesRead
                
                if(bytesRead > 0)
                {
                    inBuffer[bytesRead] = '\0';
                    printf("RECEIVED URL: %s\n", inBuffer);
                }

                if(bytesRead == 0) // if down pipe is closed
                {
                    printf("Child %d (PID %d) exiting...\n", idx, getpid());
                    exit(EXIT_SUCCESS);
                }

                if(bytesRead < 0) // error
                {
                    perror("read");
                    sleep(1);
                }

                // bytes read is valid, inBuffer contains URL
                int result = downloadToFile(inBuffer, "fileTest", 0);

                if(result) // success
                {
                    send = '#';
                    write(upPipes[idx][1], &send, 1);
                }
                else
                {
                    send = '#';
                    write(upPipes[idx][1], &send, 1);
                }
            }
            // end of child process
        }
        else // PARENT PROCESS
        {
            printf("PARENT!\n\n");
            close(downPipes[idx][0]); // close read end of pipe
            pids[idx] = pid;
        }
    }

    int URLindex = 0;
    
    //---------------------------------------------------PROCESSING LOOP:
    while(1) // PROBLEM: Never getting ready child
    {
        int readyChild = -1;

        // check up pipes for ready children
        for(int i = 0; i < max_pids; i++)
        {
            // read from up pipe of children to get next ready child
            int bytesRead = read(upPipes[i][0], readyBuffer, 1); // put pipe data into inBuffer, status into bytesRead
            
            printf("BYTES READ: %d, BUFFER: %s\n", bytesRead, readyBuffer);
            fflush(stdout);
            if(bytesRead > 0) // if ready child
            {
                readyChild = i;
                i = max_pids;
            }
            else
            {
                sleep(1);
                continue;
            }

            // inbuffer contains 
        }
    
        // if readyChild is valid index
        if(readyChild != -1)
        {
            // child is ready
            char* url = readURL(URLindex);

            if(*url == '|') // no more valid URLs
            {
                printf("NO MORE URLS!");
                fflush(stdout);
                break; // end program
            }

            write(downPipes[readyChild][1], url, buffer_size); // write to buffer for ready child
            URLindex++;
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

char* readURL(int nextIndex)
{
    FILE *filePtr = fopen(path, "r");

    static char link[buffer_size];

    if(filePtr == NULL){return ';';} // failure escape char

    int index = 0;
    while(1)
    {
        if(fgets(link, buffer_size, filePtr)) // fgets code: https://en.cppreference.com/w/c/io/fgets
        {
            // current line saved to link
            // only increment if no prefix

            if(link[0] == '#') // downloaded, success
            {
                continue;
            }
            if(link[0] == '~') // downloaded, failure
            {
                continue;
            }

            if(index == nextIndex) // the line we want
            {
                fclose(filePtr);
                return link;
            }

            index++;
        }
        else // no more lines in file
        {
            fclose(filePtr);
            return '|'; // EOF escape char
        }
    }
}

int updateURL(int index, bool success)
{
    FILE *filePtr = fopen(path, "r");
    FILE *temp = fopen("temp.txt", "w");

    char prepend = (success) ? '#' : '~';

    char line[buffer_size];

    char newLine[buffer_size];

    while (fgets(line, sizeof(line), filePtr))
    {
        if (strcmp(line, "target line\n") == 0)
        {
            snprintf(newLine, buffer_size, "%c%s", prepend, line);
            fputs(newLine, temp);
        }
        else
            fputs(line, temp);
    }

    fclose(filePtr);
    fclose(temp);
    rename("temp.txt", path);
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