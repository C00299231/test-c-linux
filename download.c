#include <stdio.h>
#include <curl/curl.h>

char links[30][100]; // this will contain file NAMES!

const char linksFile[] = "filesToDownload.txt";
const char *outFileName = "/home/user/Desktop/testOutput/";

int readLinks(char[]);

int main()
{

    // first, read the links from the file
    if(!readLinks(linksFile))
    {
        printf(stderr, "READING FAILED! Exiting...\n");
        return 1;
    }
    
    // next, download from web and save
    for(int i = 0; i < 30; i++)
    {
        if(links[i][0] == '\0') // empty line
            continue;

        const char *fullPath = getFullPath(links[i]);
        if(!downloadToFile(links[i], fullPath, i))
        {
            printf(stderr, "DOWNLOAD FAILED for %s! Continuing...\n", links[i]);
            continue;
        }
        printf("Downloaded %s to %s\n", links[i], fullPath);
    }

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

// combine output folder with file name
const char getFullPath(const char *filename)
{
    char fullPath[200];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", linksFile, filename);
    return fullPath;
}

// download and save to file
int downloadToFile(const char *url, char *filename, int index;) // curl DL code fromstackoverflow.com/questions/11471690/curl_h_no_such_file_or_directory
{
    // init curl and file
    CURL *curl = curl_easy_init();
    FILE *fp = fopen(filename, "wb");

    // errors
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        return 0; // failure
    }
    if (!fp) {
        perror("Failed to open file");
        curl_easy_cleanup(curl);
        return 0;
    }

    // set curl options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

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