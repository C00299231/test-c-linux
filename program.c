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

// NAME:    Adam Noonan
// ID:      C00299231
// DATE:    08-10-2025
// PROGRAM: Parallel file downloader

// pipe code from https://www.geeksforgeeks.org/c/pipe-system-call/
// file reading code from https://www.w3schools.com/c/c_files_read.php
// curl DL code from https://www.stackoverflow.com/questions/11471690/curl_h_no_such_file_or_directory
// fgets code from https://en.cppreference.com/w/c/io/fgets
// overwrite file line code from https://stackoverflow.com/questions/33699915/how-to-read-and-edit-specific-line-of-a-text-file-in-c

// amount of downloader processes
#define max_pids 3

// max URL size (higher may reduce performance)
#define buffer_size 128

// file where URLs are stored
#define path "filesToDownload.txt"


// init functions
char* readURL(int);
int downloadToFile(char *url, char *filename);
char* nameFromURL(char*);

int main()
{
    int parentPid = getpid();
    pid_t pids[max_pids];

    char inBuffer[buffer_size];
    char readyBuffer;
    
    // pipe stuff
    int downPipes[max_pids][2];
    int upPipes[max_pids][2];

    printf("\n- AUTO FILE DOWNLOADER -\n\n");
    

    printf("Parent PID: %d\n", getpid());

    //---------------------------------------------------CREATION LOOP:
    for(int idx = 0; idx < max_pids; idx++) // create each child
    {

        // create pipes
        pipe(downPipes[idx]);
        pipe(upPipes[idx]);
        fcntl(upPipes[idx][0], F_SETFL, O_NONBLOCK); // make upPipes non-blocking

        pid_t pid = fork();

        if(pid < 0) // failed child process
        {
            printf("child %d failed to fork...\n", idx);
        }
        
        if(pid == 0) // CHILD PROCESS
        {
            //sleep(1);
            close(downPipes[idx][1]); // close write end of down pipe
            close(upPipes[idx][0]); // close read end of up pipe
            printf("Child initialized!\n");

            // first, write ready status
            // child sends same character as URL writer for ready-success, or ready-failure
            
            // ready w/ no result, send r
            char send = 'r';

            write(upPipes[idx][1], &send, 1);
            
            while(1) // child keeps going until write end of pipe is closed
            {
                int bytesRead = read(downPipes[idx][0], inBuffer, buffer_size-1); // get pipe data into inBuffer, status into bytesRead

                printf("child %d - bytes read from parent: %d, buffer contents: %s\n", idx, bytesRead, inBuffer);

                if(bytesRead > 0)
                {
                    inBuffer[bytesRead] = '\0';
                    
                }
                if(bytesRead == 1)
                {
                    continue;
                }

                if(bytesRead == 0) // if down pipe is closed
                {
                    printf("Pipe closed! Child %d (PID %d) exiting...\n", idx, getpid());
                    close(downPipes[idx][0]);
                    close(upPipes[idx][1]);
                    exit(EXIT_SUCCESS);
                }

                if(bytesRead < 0) // error
                {
                    perror("read");
                    //sleep(1);
                }

                // bytes read is valid, inBuffer contains URL
                int result = downloadToFile(inBuffer, "fileTest.txt");

                if(result) // success
                {
                    send = '#';
                    write(upPipes[idx][1], &send, 1);
                }
                else
                {
                    send = '~';
                    write(upPipes[idx][1], &send, 1);
                }
            }
            // end of child process
        }
        else // PARENT PROCESS
        {
            close(downPipes[idx][0]); // close read end of pipe
            close(upPipes[idx][1]);
            pids[idx] = pid;
        }
    }

    int URLindex = 0;
    
    //---------------------------------------------------PROCESSING LOOP:
    while(1)
    {
        fflush(stdout);
        int readyChild = -1;

        // check up pipes for ready children
        for(int i = 0; i < max_pids; i++)
        {
            // read from up pipe of children to get next ready child
            int bytesRead = read(upPipes[i][0], &readyBuffer, 1); // put pipe data into inBuffer, status into bytesRead
            
            //printf("fromChild BYTES READ: %d, BUFFER: %c\n", bytesRead, readyBuffer);
            fflush(stdout);
            if(bytesRead > 0) // if ready child
            {
                readyChild = i;
                i = max_pids;
            }
            else
            {
                continue;
            }
        }
    
        // if readyChild is valid index
        if(readyChild != -1)
        {
            printf("Child %d is ready for a URL! sending %d...\n", readyChild, URLindex);
            fflush(stdout);
            // child is ready
            char *url = readURL(URLindex);

            if(url == '|') // no more valid URLs
            {
                printf("\nParent: No more URLs!\n\n");
                fflush(stdout);
                break; // end program
            }

            fflush(stdout);
            
            write(downPipes[readyChild][1], url, buffer_size); // write to buffer for ready child
            URLindex++;
            readyChild = -1;
        }
        else
        {
            // do nothing...
        }
    }

    fflush(stdout);

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

    printf("Parent exiting program...\n");
    return 0;
}

//=================================================================END OF MAIN



char* readURL(int nextIndex)
{
    FILE *filePtr = fopen(path, "r");

    static char url[buffer_size];

    if(filePtr == NULL){return ';';} // failure escape char

    int index = 0;
    while(1)
    {
        if(fgets(url, buffer_size, filePtr)) // fgets code: https://en.cppreference.com/w/c/io/fgets
        {
            fflush(stdout);

            // replace newline
            for(int i = 0; i < buffer_size; i++)
            {
                if(url[i] == '\n')
                {
                    url[i] = '\0';
                }
            }

            if(url[0] == '#') // downloaded, success
            {
                continue;
            }
            if(url[0] == '~') // downloaded, failure
            {
                continue;
            }

            if(index == nextIndex) // the line we want
            {
                fflush(stdout);
                fclose(filePtr);
                return url;
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

int downloadToFile(char *url, char *filename)
{
    char* name;
    name = nameFromURL(url);
    printf("Attempting download of %s.\n", name);

    if(url[0] == '\0') // empty URL
    {
        return 0;
    }
    
    // init curl and file
    CURL *curl = curl_easy_init();
    FILE *fp = fopen(name, "wb");

    // errors
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        return 0; // failure
    }
    if (!fp)
    {
        printf("Failed to open file\n");
        curl_easy_cleanup(curl);
        return 0;
    }

    // set curl options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); // find reference for writedata
    
    CURLcode res = curl_easy_perform(curl);
    fflush(fp);
    fclose(fp);

    // more errors
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return 0;
    }
    printf("Successful download of %s!\n\n", name);
    curl_easy_cleanup(curl);
    return 1; // success
}

char* nameFromURL(char* url) // edited from https://www.geeksforgeeks.org/c/get-a-substring-in-c/
{
    static char newName[buffer_size];
    if(url[0] == '\0')
    {
        return "e";
    }

    int charPos;

    for(int i = buffer_size-1; i >= 0; i--)
    {
        if(url[i] == '/')
        {
            charPos = i+1;
            break;
        }
    }

    strncpy(&newName, url + charPos, buffer_size-charPos);


    return &newName;

}