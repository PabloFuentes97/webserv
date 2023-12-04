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
<<<<<<< HEAD
#include <sstream>
#include <map>
#include <dirent.h>
#include <cstdio>
#include <array>
#include <vector>
#define PORT 8080
#define BUF_SIZE 2000
=======
#include <string.h>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <sys/event.h>
#include <vector>
#include <list>
#include <map>
#include <sstream>
#include <fstream>
>>>>>>> af04e7393858c5426766403aa0afcd6441fe0011


struct HttpRequest {
    std::string method;
    std::string url; //version
    std::string body;
    std::map<std::string, std::string> headers;

	int status;
};

<<<<<<< HEAD
struct client {
    int fd;
	HttpRequest request;
	//server *SocketServer;
};
=======
typedef struct bTreeNode //sustituir todos los tipos complejos y contenedores por estructuras de C - arrays, listas enlazadas,
//evitar que haga copias innecesarias al añadir elementos a un contenedor
{
	std::string					contextName;
	size_t						contextType;
	size_t						contextOperation;
	std::vector<std::string>	contextArgs;
	//cambiar contenido a <pair>: key-value, pero valores puede ser una lista, múltiples valores
	std::vector<std::string>	keys;
	std::vector<std::string>	values;
	std::vector<int>	mapKeyValues;
	//std::map<std::string, std::string>	directivesMap;
	//std::vector<std::pair<std::string, std::vector<std::string> > >	directives;
	std::vector<bTreeNode*>		childs;
} bTreeNode;

//más "orientado a objetos" cada token un objeto con sus atributos "individuales"
typedef struct	s_token
{
	size_t		line;
	size_t		type;
	std::string	value;
}	t_token;

//más "orientado a datos" una estructura que guarda en arrays las propiedades de todos los tokens, se "mapea" de manera lineal, unívoca
typedef struct s_tokens
{
	int		n;
	size_t	*line;
	size_t	*type;
	std::vector<std::string>	values;
}	t_tokens;
>>>>>>> af04e7393858c5426766403aa0afcd6441fe0011

class	clientQueue {

<<<<<<< HEAD
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
=======
//data structures transforms
char	*seLstToStr(seLst &lst);
>>>>>>> af04e7393858c5426766403aa0afcd6441fe0011

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
