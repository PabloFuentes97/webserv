#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
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
#include <list>

#define MAX_SE_ELEM 64
#define MAX_READ 17

typedef struct seNode{
	size_t			elem_n;
	char 			elem[MAX_SE_ELEM];
	int				content_bytes;
	struct seNode *next;
} seNode;

typedef struct	seLst{
	seNode *head;
	seNode *tail;
	size_t	size;
	size_t	bytes;
} seLst;

struct HttpRequest {
    std::string method;
    std::string url; //version
    std::string body;
    std::map<std::string, std::string> headers;

	int status;
};

struct client {
    int fd;
	int	serverID;
	HttpRequest request;
	//server *SocketServer;
};

struct HttpResponse {
	std::string firstLine; //method, version
    std::string body; //if empty, content-length
    //std::map<std::string, std::string> headers;

	//constructor y métodos
};

class	clientQueue {

	public:
		void clearRequest(int);
		void addClient(int, int);
		clientQueue();
		~clientQueue();
		std::vector<client> clientArray;
		int getPos(int);
};

enum	contextType{main_c, events, http, server, location};

typedef union context_types{
		void	**matrix;
		void	*array;
		seLst	lst;	
	}	context_types;

typedef struct	context{
	char						*contextName;
	size_t						contextType;
	size_t						contextOperation;
	std::vector<std::string>	contextArgs;
	//cambiar contenido a <pair>: key-value, pero valores puede ser una lista, múltiples valores
	std::vector<std::string>	keys;
	std::vector<std::string>	values;
	std::vector<int>	mapKeyValues;
	//std::map<std::string, std::string>	directivesMap;
	std::vector<std::pair<std::string, std::vector<std::string> > >	directives; //hacerlo asi, no usar el vector de mapear valores con keys
} context;

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
	std::vector<std::string>	childsNames; //nombre del tipo de cada hijo añadido
	std::vector<std::string>	allSubContexts; //nombres de todos los posibles subcontextos que puede tener
	void						*_content;
	std::vector<bTreeNode*>		childs; //punteros a los hijos
	bTreeNode(){

	}; //constructor por defecto
	bTreeNode(void *content)
	{

	}; //constructor con parámetros
} bTreeNode;

typedef struct	servers{
	
	int			*serversFd;
	int			servers_n;
	clientQueue	clients;
	std::vector<bTreeNode*>	serversPtr;
}	servers ;

/*typedef struct	Servers
{
	int			*serverSockets; // array de sockets de escucha
	int			*connSockets; //array de sockets de conexión
	int			*mapconnListen; //array con el socket de escucha del servidor al que esta ligado cada socket de conexión con cliente
	char 		***requests; //triple puntero char a matrices de solicitudes de datos, también se mapean a sockets de conexión
	bTreeNode	**serversPtr; //array de punteros al nodo de cada servidor en el árbol
	int			*status; //array con el estado de la conexión del socket de conexión

	EJEMPLO:
		Programa que maneja 3 servidores: 
		Sockets de escucha: [3, 4, 5]; se abren al principio del programa
		Sockets de conexión: [6, 7, 8, 9, 10, ...]; número indefinido, socket por cada cliente
		Mapear sockets de conexión a sockets de escucha: [3, 3, 4, 3, 5, ...]; número indefinido, pero el mismo que sockets de conexión:
			a cada socket de conexión le corresponde un id al socket de escucha del servidor al que pertenece
		Mapear requests a sockets de conexión: [request1, request2, request3, request4, request5, ...]
	

}	Servers;*/

enum	token_type{word, openBracket, closeBracket, endDeclare}; //enums para tipos de token

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

//seLst functions
seNode	*newseNode();
void	seLstPushBack(seLst &lst, seNode *newNode);
void	seLstFree(seLst &lst);

//data structures transforms
char	*seLstToStr(seLst &lst);

//str functions
size_t	countCharinStr(char *str, char c);

//parse files
char	*readFileSeLst(int fd);
void	parseFile(int fd);

//socket funcs