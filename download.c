#include <stdio.h>
#include <curl/curl.h>

char links[30][100];

const char linksFile[] = "filesToDownload.txt";

int readLinks(char[]);

int main() // curl DL code fromstackoverflow.com/questions/11471690/curl_h_no_such_file_or_directory
{
    if(!readLinks(linksFile))
    {
        printf(stderr, "READING FAILED! Exiting...\n");
        return 1;
    }

    CURL *curl;
    FILE *fp;
    CURLcode res;

    const char *outFileName = "/home/user/Desktop/testOutput";

    curl = curl_easy_init();
    if(!curl)
    {
        printf(stderr, "CURL INIT FAILED!");
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, links[0]);

    res = curl_easy_perform(curl);

    if(res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);

    return 0;
}

int readLinks(char path[]) // file reading code from www.w3schools.com/c/c_files_read.php
{
    FILE *filePtr = fopen(path, "r");

    if(filePtr == NULL)
    {
        // no file!
        printf("\nUnable to open the specified file.\n");
        return 0;
    }

    int currentLine = 0;

    while(fgets(links[currentLine], 100, filePtr) && currentLine < 29)
    {
        // stored string!
    }


    fclose(filePtr);

    return 1;
}

int writeToFile(const char *filename, const char *data)
{
    FILE *filePtr = fopen(filename, "w");
    if(filePtr == NULL)
    {
        // no file!
        printf("\nUnable to open the specified file.\n");
        return 0;
    }
    fputs(content, filePtr);
    fclose(filePtr);
    return 1; // success
}

const char getFullPath(const char *filename, const char *directory)
{
    char fullPath[200];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", directory, filename);
    return fullPath;
}