/*
Billy Kwong
bqkwong
CSE130 Fall 2019
ASGN 1
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <vector>
#include <string>
#include <bits/stdc++.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

extern int errno;

#define BUFFER_SIZE 4096
#define DEFAULT_PORT 80

void exit_with_error(const char * error_message);
int is_number_of_arguments_valid(int number_of_arguments);
bool is_valid_file(string str);
bool isCapital(char ch);
bool isSmall(char ch);
bool isNumber(char ch);
bool isSpecial(char ch);


int main(int argc, char * argv[])
{
	if (argc < 2 || argc > 3) 
	{
		exit_with_error("Usage: ./httpserver <ip> <port> or ./httpserver <ip>.");
	}
	int port = DEFAULT_PORT;
	if(argc == 3)
	{
		port = atoi(argv[2]);
	}
	
	int server_sockfd;    // socket on which server process will listen for incoming con
   	int client_sockfd;    // socket on which the server will be comm with the client
   	struct sockaddr_in server_addr;
   	struct sockaddr_in client_addr;

	// Create passive socket for the server
   	server_sockfd = socket (AF_INET, SOCK_STREAM, 0);

	//***  Create an address structure containing server IP addr
	//***  and port, then bind the server_sockfd with this address
   	
   	server_addr.sin_family = AF_INET;
   	server_addr.sin_port = htons(port);
   	inet_aton(argv[1], &server_addr.sin_addr);
   	memset(&(server_addr.sin_zero), '\0', sizeof(server_addr.sin_zero));
   	bind(server_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	
	//*** Create a connection queue and wait for clients
	listen(server_sockfd, SOMAXCONN);
	
	
	while(1)
	{
      		fprintf(stderr,"\nServer waiting for client connection...");
      		
		//***	Accept a connection, blocks until connection from client is established
		//****	will return a brand new descriptor for comm with this single connection
	      	socklen_t client_len = sizeof(client_addr);
	      	client_sockfd=accept(server_sockfd,(struct sockaddr*)&client_addr,&(client_len));
	      	if(client_sockfd == -1 )
	      	{
	      		char er[] = "HTTP/1.1 500 INTERNEL SERVER ERROR\r\n";
	      		send(client_sockfd, er, sizeof(er),0);
   			continue;
   		}
   		else
   		{	
      			fprintf(stderr, "\n********* CLIENT CONNECTION ESTABLISHED ********");
		}
		vector<string> header;
		int count = 0;
			
      		//***** Read from sockfd and write back
      		char buf[BUFFER_SIZE];
      		int rv;
      		while ((rv = recv(client_sockfd, buf, sizeof(buf), 0)))
      		{
      			char* token = strtok(buf, "\n");
			while (token != NULL)
			{
				count++;
				header.push_back(string(token));
				token = strtok(NULL, "\n");
			}
      		}
   		
   		// Vector of string to save tokens 
    		vector <string> tokens; 
      			
    		stringstream ss(header[0]); 
      
    		string intermediate;
      
    		// Tokenizing w.r.t. space ' ' 
    		while(getline(ss, intermediate, ' ')) 
    		{ 
        		tokens.push_back(intermediate); 
    		} 
   		
   		if(tokens.size() == 3 && tokens[0] == "GET" && tokens[2] == "HTTP/1.1" && is_valid_file(tokens[1]))
   		{
   			if(access(tokens[1].c_str(), F_OK)==-1)
   			{
   				char er[] = "HTTP/1.1 404 NOT FOUND\r\n";
   				send(client_sockfd, er, sizeof(er),0);
   			}
   			else if(access(tokens[1].c_str(), R_OK)==-1)
   			{
   				char er[] = "HTTP/1.1 403 FORBIDDEN\r\n";
   				send(client_sockfd, er, sizeof(er),0);
   			}
   			else
   			{
   				int fd = open(tokens[1].c_str(),O_RDONLY);
   				size_t bytes_read;
   				while ((bytes_read = read (fd, buf, BUFFER_SIZE))) //read line from file and print it
				{
					send(client_sockfd, buf, bytes_read,0);
				}
				close(fd);
   				char er[] = "HTTP/1.1 200 OK\r\n";
   				send(client_sockfd, er, sizeof(er),0);			
			}	
   		}
   		else if(tokens.size() == 3 && tokens[0] == "GET" && tokens[2] == "HTTP/1.1" && is_valid_file(tokens[1]))
   		{
   			int fd = open(tokens[1].c_str(),O_RDWR & O_CREAT);
   			if(fd == -1)
   			{
   				char er[] = "HTTP/1.1 403 FORBIDDEN\r\n";
   				send(client_sockfd, er, sizeof(er),0);
   			}
   			else
   			{
   				vector<string> content;
   				stringstream cl(header[1]);
   				while(getline(cl, intermediate, ' ')) 
    				{ 
        				content.push_back(intermediate); 
    				}
    				if(content[0] == "Content-Length:")
    				{
    					int len = atoi(content[1].c_str());
    					int i=2;
    					while(len>0 && i<count)
    					{
    						write(fd,header[i].c_str(),header[i].length()+1);
    						len -= header[i].length();
    						i++;
    					}
    				}
    				else
    				{
    					for(int i=1;i<count;i++)
    					{
    						if(header[i] != "\r")
    						{
    							write(fd,header[i].c_str(),header[i].length()+1);
						}
    					}
    				}
    				close(fd);
    				char er[] = "HTTP/1.1 201 CREATED\r\n";
   				send(client_sockfd, er, sizeof(er),0);
   			
   			}
   		}
   		else
   		{
   			char er[] = "HTTP/1.1 400 BAD REQUEST\r\n";
   			send(client_sockfd, er, sizeof(er),0);
   		}
        	close(client_sockfd);
      		fprintf(stderr, "\n********* CLIENT CONNECTION TERMINATED ********");
      	}
	
	return 0;
}



void exit_with_error(const char * error_message) 
{
	fprintf(stderr, "%s\n", error_message);
	exit(-1);
}


bool is_valid_file(string str)
{
	if(str.length() > 27)
		return false;
	for (int i = 0; i < str.length(); i++)
	{
		if (!(isSmall(str[i]) || isCapital(str[i]) || isNumber(str[i]) || isSpecial(str[i])))
			return false;
	}
	return true;
}

bool isCapital(char ch)
{
	if (ch >= 'A' && ch <= 'Z')
		return true;
	return false;
}

bool isSmall(char ch)
{
	if (ch >= 'a' && ch <= 'z')
		return true;
	return false;
}

bool isNumber(char ch)
{
	if (ch >= '0' && ch <= '9')
		return true;
	return false;
}

bool isSpecial(char ch)
{
	if (ch == '-' || ch == '_')
		return true;
	return false;
}

