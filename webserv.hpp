#pragma once

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>   
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <dirent.h>
#include <cstdio>
#include <array>
#include <vector>
#define PORT 8080
#define BUF_SIZE 2000


struct HttpRequest {
    std::string method;
    std::string url; //version
    std::string body;
    std::map<std::string, std::string> headers;

	int status;
};

struct client {
    int fd;
	HttpRequest request;
	//server *SocketServer;
};

class	clientQueue {

	public:
		void clearRequest(int);
		void addClient(int);
		clientQueue();
		~clientQueue();
		std::vector<client> clientArray;
		int getPos(int);
};

struct HttpResponse {
	std::string firstLine; //method, version
    std::string body; //if empty, content-length
    //std::map<std::string, std::string> headers;

	//constructor y métodos
};

//--HTTP REQUEST---
HttpRequest loadRequest(char *buffer);
std::string getRequestedFile(HttpRequest *currentRequest);
std::string getResponseBody(std::string fileToReturn);
std::string	getStatus(int status);
std::string getResponseFirstLine(HttpRequest currentRequest, std::string body);
std::string GetResponse(HttpRequest *request);
std::string ResponseToMethod(HttpRequest *request);

//---SOCKET---
void setNonBlocking(int fd);
int	getServerSocket(sockaddr_in *addr);
void	bindAndListen(int sock, sockaddr_in *addr);

//---CGI---
std::string getCgi(std::string script);

