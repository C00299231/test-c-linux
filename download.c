#include <stdio.h>
#include <curl/curl.h>

// STEPS TO PERFORM:
// get link from file
// download file from internet, save data to new file

char links[30][100]; // this will contain file NAMES!

const char linksFile[] = "filesToDownload.txt";
const char *outFileName = "/home/user/Desktop/testOutput/";

int readLinks(const char[]);
int writeToFile(const char*, char[30][100]);
int downloadToFile(const char*, char*, int);

int main()
{

    // first, read the links from the file
    if(!readLinks(linksFile))
    {
        printf("READING FAILED! Exiting...\n");
        return 1;
    }
    
    // next, download from web and save
    for(int i = 0; i < 30; i++)
    {
        if(links[i][0] == '\0') // empty line
            continue;

        
            char newFileName[] = "fileX";
            newFileName[4] = i+'0';
        
        if(!downloadToFile(links[i], newFileName, i))
        {
            fprintf(stderr, "DOWNLOAD FAILED for %s! Continuing...\n", links[i]);
            continue;
        }
        printf("Downloaded %s to %s\n", links[i], newFileName);
    }

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

/*
// combine output folder with file name
const char getFullPath(const char *filename)
{
    char fullPath[200];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", linksFile, filename);
    return fullPath;
}*/

// download and save to file
int downloadToFile(const char *url, char *filename, int index) // curl DL code fromstackoverflow.com/questions/11471690/curl_h_no_such_file_or_directory
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