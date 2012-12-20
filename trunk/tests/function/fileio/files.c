#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>

int file_select(struct direct * entry)
{
    if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
        return 0;
    else
        return 1;
}

int main(int argc, char* argv[])
{
    char pathname[PATH_MAX];
    printf("getcwd = %s \n", getcwd(pathname, PATH_MAX) ); 

    int count,i;
    struct direct **files;
    //count = scandir(pathname, &files, file_select, alphasort);
    count = scandir(pathname, &files, NULL, NULL);
 
    if (count < 0)
        printf("No files in this directory \n");
    else
        for (i=0; i<count; i++)
             printf("%d: %s \n", i, files[i]->d_name);

    return 0;
}
