/* Billy Kwong
   bqkwong
   CSE130 F19
   ASGN0
*/

#include<stdio.h> 
#include<fcntl.h> 
#include<err.h> 
#include<errno.h> 
#include<stdlib.h> 
#include<string.h> 
#include<unistd.h>

#define BUFFER_SIZE 32
extern int errno; 
int main(int argc, char* argv[]) {
	char buffer[BUFFER_SIZE];
	
	if (argc == 1) //no command line parameters
	{
		while (1) //read line from standard input and print it on standard output
		{
			size_t bytes_read = read (0, buffer, BUFFER_SIZE);
			if(bytes_read <= 0) //no data is read
				break;
			write(1, buffer, bytes_read);
		}
		return 0;
	}
	for (int i=1; i<argc; i++) //go through all command line parameters except the first one
	{	
		//if file name is -
		if(!strcmp(argv[i],"-"))
		{
			while (1) //read line from standard input and print it on standard output
			{

				size_t bytes_read = read (0, buffer, BUFFER_SIZE);
				if(bytes_read <= 0) //no data is read
					break;
				write(1, buffer, bytes_read);
			}
		}
		else
		{
			//open file and get descriptor
			int fd = open(argv[i], O_RDONLY);
			//if file failed to open
			if(fd == -1)
			{
				warn("%s", argv[i]); //print error 				
				exit(0);
			}
			while (1) //read line from file and print it on standard output
			{
				size_t bytes_read = read (fd, buffer, BUFFER_SIZE);
				if(bytes_read <= 0) //no data is read
					break;
				write(1, buffer, bytes_read);
			}
			//close file
			close(fd);
		}
	}
	return 0;
}
