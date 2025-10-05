#include <stdio.h>
#include <curl/curl.h>

// STEPS TO PERFORM:
// get link from file
// download file from internet, save data to new file

char links[30][100];

const char linksFile[] = "filesToDownload.txt";

int readLinks(const char[]);
int writeToFile(const char*, char[30][100]);

int main() // curl DL code fromstackoverflow.com/questions/11471690/curl_h_no_such_file_or_directory
{
    if(!readLinks(linksFile)) // get web links from file
    {
        printf("READING FAILED! Exiting...\n");
        return 1;
    }
    const char *outFileName = "/home/user/Desktop/testOutput/";

    CURL *curl;
    FILE *fp = fopen("newFile", "wb");

    
    // initialize curl
    curl = curl_easy_init();
    if(!curl)
    {
        printf("CURL INIT FAILED!");
        return 1;
    }
    
    // set curl options, perform curl
    curl_easy_setopt(curl, CURLOPT_URL, links[0]);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    CURLcode res = curl_easy_perform(curl);

    if(res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);

    return 0;
}

int readLinks(const char path[]) // file reading code from www.w3schools.com/c/c_files_read.php
{
    FILE *filePtr = fopen(path, "r");

    if(filePtr == NULL)
    {
        // no file! failure
        printf("\nUnable to open the specified file.\n");
        return 0;
    }

    int currentLine = 0;

    while(fgets(links[currentLine], 100, filePtr) && currentLine < 29)
    {
        // stored string! nothing else to do here :/
    }
    
    fclose(filePtr);

    return 1;
}

int writeToFile(const char *filename, char data[30][100])
{
    FILE *filePtr = fopen(filename, "w");
    if(filePtr == NULL)
    {
        // no file!
        printf("\nUnable to open the specified file.\n");
        return 0;
    }

    for(int idx = 0; idx < 30; idx++)
    {
        fprintf(filePtr, "%s\n", data[idx]);
    }

    fclose(filePtr);

    return 1; // success
}