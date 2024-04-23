#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>        // creat(~, S_IRUSR) S_IRUSR define in sys/stat.h
#define buffersize 4026     
#define copymode   0644     // 110 100 100  rw-r--r--

void oops(char *, char *);

int main(int ac, char *av[])
{
    int in_fd, out_fd, n_chars;
    char buf[buffersize];

    if(ac != 3)
    {
        fprintf(stderr, "usage: %s source destination\n", *av);
        exit(1);
    }
    
    if( (in_fd = open(av[1], O_RDONLY)) == -1)
    {
        oops("Cannot open ", av[1]);
        exit(1);
    }

    if( ( out_fd = creat(av[2], copymode)) == -1)
    {
        oops("Cannot open", av[2]);
        exit(1);
    }

    while( (n_chars = read(in_fd, buf, buffersize)) > 0 )       // read 버퍼에 읽은만큼 저장하고
    {
        if( write(out_fd, buf, n_chars) != n_chars)             // 버퍼 내용을 write
        {
            oops("Write error to ", av[2]);
        }
    }
    if(n_chars == -1)
    {
        oops("Read error from ", av[1]);
    }

    if(close(in_fd) == -1 || close(out_fd) == -1)
    {
        oops("Error closing files", "");
    }

    return 0;
}


void oops(char *s1, char *s2)
{
    fprintf(stderr, "Error : %s", s1);
    perror(s2);
    exit(1);
}