/*
Billy Kwong
bqkwong
CSE130 Fall 2019
ASGN 3
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
#include <netdb.h>
#include <arpa/inet.h>

using namespace std;

extern int errno;

#define DEFULT_PORT 8080
#define BUFFER_SIZE 4096

int logging = 0;


void log(const char *format, ...)
{
    if(logging)
    {
        va_list args;

        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

typedef struct cache_item 
{
    int init;
    string resource_key;
    string resource_data;
} cache_item_t;

int _pointer;
cache_item_t _cache_item[4];

string* cache_find(string resource_key)
{
    for(int i=0; i<4 ; i++)
    {
        if( _cache_item[i].init == 1 )
        {
            if( _cache_item[i].resource_key.compare(resource_key) == 0  ) 
            {
                return &_cache_item[i].resource_data;   
            }
        }
    }
    return NULL;
}


void cache_add(string resource_key, string resource_data)
{
    for(int i=0; i<4 ; i++)
    {
        if( _cache_item[i].init == 0 )
        {
            _cache_item[i].init = 1;
            _cache_item[i].resource_key.clear();
            _cache_item[i].resource_data.clear();
            _cache_item[i].resource_key.append(resource_key);
            _cache_item[i].resource_data.append(resource_data);
            return;            
        }
    }

    if(_pointer==4)
        _pointer=0;
    
    _cache_item[_pointer].resource_key.clear();
    _cache_item[_pointer].resource_data.clear();
    _cache_item[_pointer].resource_key.append(resource_key);
    _cache_item[_pointer].resource_data.append(resource_data);

    _pointer++;
}


int main(int argc, char const *argv[])
{
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int port = DEFULT_PORT;


    char *ROOT = getenv("PWD");
    
    for (int idx = 1; idx < argc ; idx++) 
    {
        
        if(argv[idx][0] == '-')
        {
            switch (argv[idx][1]) 
            {
                case 'c': 
                    logging = 1; 
                    break;
                case 'p': 
                    port = atoi(argv[idx+1]); 
                    break;
                default:
                    cout << "Usage: ./httpserver -p <port> or ./httpserver <port> -c\n" << endl;
                    exit(EXIT_FAILURE);
            }
        }   
    }   

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    memset(address.sin_zero, '\0', sizeof address.sin_zero);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    else
    {
        log("Server is started on port %d\n", port );
    }
    
    while (1)
    {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        char* http_method, *http_version, *http_resource, buffer[BUFFER_SIZE] = {0};
        
        vector<string> header_lines;
		int header_lines_count = 0;
        read(client_socket, buffer, BUFFER_SIZE);

        char *token = strtok(buffer, "\n");
        while (token != NULL)
        {
            header_lines_count++;
            header_lines.push_back(string(token));
            token = strtok(NULL, "\n"); 
        }
		

        http_method = strtok(buffer, " \t\n");
        http_resource = strtok(NULL, " \t");
        http_version = strtok(NULL, " \t\n");


        if (  strncmp(http_version, "HTTP/1.1", 8) != 0 ) 
        {
            log("Error# 400, Bad Request.\n" );
            write(client_socket, "HTTP/1.0 400 Bad Request\n", 25);
        }
        else if( strncmp(http_method, "GET", 3) == 0  )
        {
            char data_to_send[BUFFER_SIZE], path[BUFFER_SIZE];
            int fd, bytes_read;

            if (strncmp(http_resource, "/\0", 2) == 0)
            {
                log( "GET Request for home page.\n" );
                http_resource = (char *)"/index.html";
            }

            strcpy(path, ROOT);
            strcpy(&path[strlen(ROOT)], http_resource);
            
            if(cache_find(path) != NULL)
            {
                log("GET Request, Sending page %s\n", path );

                string* cached = cache_find(path);
                send(client_socket, "HTTP/1.0 200 OK\n\n", 17, 0);
                write(client_socket, cached->c_str(), cached->length());

                log("GET %s length %d [was in cache]\n", path, cached->length());
                
            }
            else
            {
                if ((fd = open(path, O_RDONLY)) != -1) //FILE FOUND
                {
                    log( "GET Request, Sending page %s\n", path );

                    string data_to_cached;
                    send(client_socket, "HTTP/1.0 200 OK\n\n", 17, 0);
                    while ((bytes_read = read(fd, data_to_send, BUFFER_SIZE)) > 0)
                    {
                        write(client_socket, data_to_send, bytes_read);
                        data_to_cached.append(data_to_send);
                    }
                    cache_add(path, data_to_cached);

                    log("GET %s length %d [was not in cache]\n",path, data_to_cached.length());
                    
                }
                else
                {
                    log("Error# 404 File not found\n" );
                    write(client_socket, "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
                }
            }
            
            
        }
        else if( strncmp(http_method, "PUT", 3) == 0  )
        {
            char path[BUFFER_SIZE];
            strcpy(path, ROOT);
            strcpy(&path[strlen(ROOT)], http_resource);

            log("PUT Request for %s\n", path );
            

            fstream file(path, ios::out | ios::in | ios::binary | ios::trunc);
			if (!file.is_open())
			{
				char er[] = "HTTP/1.1 403 FORBIDDEN\r\n";
				write(client_socket, er, sizeof(er));
			}
			else
			{                
                int content_length=0;
				for (int i = 0; i < header_lines_count; i++)
                {
                    char *ptr = strtok((char*)header_lines[i].c_str(), ":");
                    
                    if(  strncmp( ptr , "Content-Length", 14 ) == 0 )
                    {
                        ptr = strtok(NULL, ":");
                        content_length = atoi(ptr);
                    }

                    if(content_length>0)
                    {
                        ssize_t read_bytes;
                        char er[] = "HTTP/1.1 201 CREATED\r\n";
   				        send(client_socket, er, sizeof(er),0);
                        do
                        {
                            read_bytes = read(client_socket, buffer, BUFFER_SIZE);
                            if(read_bytes == -1 )
                            {
                                perror("Read Error");
                                exit(EXIT_FAILURE);
                            }
                            else
                            {
                                file << buffer;
                                content_length -= read_bytes;
                            }
                        }
                        while(read_bytes > 0 && content_length!=0 );
                    }

                }
                log("PUT Request File created %s\n", path );

                file.close();
				char er[] = "HTTP/1.1 201 CREATED\r\n";
				write(client_socket, er, sizeof(er));
			}
        }
        close(client_socket);
    }
    return 0;
}
