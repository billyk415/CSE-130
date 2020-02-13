Billy Kwong

bqkwong

ASGN3

README.md


# Requirements

You need a ubuntu environment with clang installed on it to run the code properly.

# Limitations and Issues

The program will only run correctly in unix environment as program using some system calls available in unix only.
Program should not run on a port already reserved by Unix for some other utility.
The IP address that user provides should be valid.
Compiling:
	make  		to compile http server from source file
	make clean	remove intermediate files 
	
Usage:
	./httpserver -p <port> -c
	
Options:
	-p <port>	Specify the port default port is 8080
	-c			Enable logging. 
