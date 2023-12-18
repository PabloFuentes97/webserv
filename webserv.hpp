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
#include <algorithm>

#define MAX_SE_ELEM 64
#define MAX_READ 17
#define BUF_SIZE 200000
#define PORT 8080


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
	std::string headerBuf;
	std::string bodyBuf;
    std::string method;
    std::string url; //version
    std::string body;
    std::multimap<std::string, std::string> headers;
	std::multimap<std::string, std::string>	bodyData;
	int status;
};

struct client {
    int fd;
	int	serverID;
	int	state; // 0 - tiene que leer header, 1 - tiene que leer body, 2 - tiene que escribir
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
		int getServerId(int);
};

enum	contextType{main_c, events, http, server, location};

/*typedef union context_types{
		void	**matrix;
		void	*array;
		seLst	lst;	
	}	context_types;

typedef struct	keyValue //utilizarla como contenido de lista o vector de claves-valor
{
	typedef	union	keyTypes
	{
		char	*str;
		int		*arrInt;
	};
	typedef	union	valueTypes
	{
		char	**valMatrix;
		//t_list	*valList;
		//std::list<std::string>	valList;
		//std::vector<std::string>	valVector;
	};
	keyTypes	key;
	valueTypes	values;
};*/
/*typedef struct	s_char_matrix{
	char	*str;
	int		*indexes;
	int		n;
}	t_char_matrix;*/

typedef struct	context{
	std::string					_name;
	size_t						contextType;
	size_t						contextOperation;
	std::vector<std::string>	_args;
	std::vector<std::pair<std::string, std::vector<std::string> > >	directives;
} context;


typedef struct bTreeNode //sustituir todos los tipos complejos y contenedores por estructuras de C - arrays, listas enlazadas,
//evitar que haga copias innecesarias al añadir elementos a un contenedor
//plantearse hacer una clase template
//llamarlo n-tree o m-tree no es un b-tree realmente
{
	std::string					contextName;
	size_t						contextType;
	std::vector<std::string>	contextArgs;
	//cambiar contenido a <pair>: key-value, pero valores puede ser una lista, múltiples valores
	std::multimap<std::string, std::string>	directivesMap;
	std::vector<std::pair<std::string, std::vector<std::string> > >	directives;
	std::vector<std::string>	childsNames; //nombre del tipo de cada hijo añadido
	//int							*childsTypes; //tipos de cada hijo, id para saber donde moverse
	void						*_content;
	std::vector<bTreeNode*>		childs; //punteros a los hijos
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
size_t	countCharinStr(const char *str, char c);

//parse files
bool		tokenizeFile(const char *file, std::vector<t_token> &tokens, std::string &del);
char		*readFileSeLst(int fd);
bTreeNode	*parseFile(char	*file);

//tree funcs - find, search

void	findNode(bTreeNode *root, bTreeNode **find_node, std::string find);
bTreeNode	*findLocation(bTreeNode *server, std::string	&URL);
bool	findFile(std::string &dirFind, std::string &file);
bool	getValue(std::vector<std::pair<std::string, std::vector<std::string> > > keyValues, std::string key, std::vector<std::string>	*values_out);


//socket funcs

//sort-insert funcs
int		binarySearch(std::vector<bTreeNode*> &vec, bTreeNode *insert);
void	binaryInsert(std::vector<bTreeNode *> &vec, bTreeNode *insert);

//directories-locations
int	cmpDirectories(std::string &s1, std::string &s2);
int	cmpLocations(bTreeNode *loc, bTreeNode *cmp);
int	cmpDirectives(void *loc, void *cmp);

//--HTTP REQUEST---
int		readEvent(struct kevent *cli, struct kevent *client_event);
//int		readEvent(clientQueue &Queue, struct kevent *client, struct kevent *client_event, int kq);
void	writeEvent(bTreeNode *server, struct kevent *cli, struct kevent *client_event);
//void	writeEvent(bTreeNode *server, clientQueue &Queue, int ident, struct kevent *client_event, int kq);
//HttpRequest loadRequest(char *buffer);
void	loadRequest(HttpRequest *request);
std::string	getRequestedFile(bTreeNode	*server, client *client);
std::string getResponseBody(std::string fileToReturn);
std::string	getStatus(int status);
std::string getResponseHeader(HttpRequest &currentRequest, std::string &body);
std::string GetResponse(bTreeNode	*server, std::string &url);
std::string ResponseToMethod(bTreeNode *server, client *client);

//---SOCKET---
void setNonBlocking(int fd);
int	getServerSocket(sockaddr_in *addr, int port);
void	bindAndListen(int sock, sockaddr_in *addr);

//---CGI---
std::string getCgi(std::string script);