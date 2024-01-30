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
#include <libc.h>

#define MAX_SE_ELEM 64
#define MAX_READ 17
#define BUF_SIZE 200000
#define PORT 8080
#define CGITIMEOUT 3

enum statusCodes {CREATED = 201,
				MOVED_PERMANENTLY = 301,
				BAD_REQUEST = 400,
				FORBIDDEN = 403,
				NOT_FOUND = 404,
				METHOD_NOT_ALLOWED = 405,
				TIMEOUT = 408,
				CONFLICT = 409,
				PAYLOAD_TOO_LARGE = 413,
				TOO_MANY_REQUESTS = 429,
				INTERNAL_SERVER_ERROR = 500,
				NOT_IMPLEMENTED = 501,
				BAD_GATEWAY = 502,
				GATEWAY_TIMEOUT = 504
};

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

typedef struct	s_context{
	std::string							_name;
	typedef enum types{MAIN, MIME_TYPES, HTTP, SERVER, LOCATION} types;
	short								_type;
	short								_op;
	std::vector<std::string>			_args;
	std::multimap<std::string, std::string>	_dirs;
} t_context;

typedef struct parseTree 
{
	t_context					context;
	std::vector<parseTree *>	childs; //punteros a los hijos
} parseTree ;

struct HttpResponse {
	std::string firstLine; //method, version
    std::string body; //if empty, content-length
	std::string	response;
    //std::map<std::string, std::string> headers;
	size_t	bytesSent;

	//constructor y métodos
};

typedef struct	chunk{
	bool		isChunked; // If true the request body is chunk encoded
	bool		complete; // If true DeChunking complete
	bool		readingSize; // If true we are currently reading the HEX value of the chunk size
	size_t		size; // Size of the current chunk
	size_t		index; // Extra bytes from the body that are not content (necesarios para mirar max body length)
	std::string	stringHex; // String of the HEX value of the chunk size
	std::string	buf; // Content
}	chunk;

typedef struct HttpRequest {
	
	chunk		chunk;
	std::string	buf;
	size_t		bufLen;
	std::string	header;
    std::string method;
	int			method_int;
	typedef enum methods{GET, POST, PUT, DELETE} methods;
    std::string url; //version
	std::string query;
    std::multimap<std::string, std::string> headers;
	//std::multimap<std::string, std::string>	bodyData;
	bool	cgi;
	std::map<std::string, std::string>	cookies;
	int status;
} HttpRequest;

typedef struct s_ports{
	std::vector<int>	id; // id del puerto
	std::vector<int>	fd; //fd del socket asociado a ese puerto
	size_t				n;
} t_ports ;


typedef struct s_events{
	pollfd	events[SOMAXCONN + 1];
	size_t	n;
	size_t	sig;
}	t_events;

typedef struct client {
    int 			fd;
	int				portID; //port id
	int				state; // 0 - tiene que leer header, 1 - tiene que leer body, 2 - tiene que escribir
	parseTree 		*server; //que cada cliente tenga un puntero a su servidor, para no tener que pasarlo como parametro por funciones
	HttpRequest 	request;
	HttpResponse	response;
	parseTree		*loc;
	pollfd 			*events[2];
	size_t			timer;
	bool 			operator==(struct client const &cmp) const
	{
		if (this->fd == cmp.fd)
			return (1);
		return (0);
	}
} client;

typedef struct	servers{
	
	int			*serversFd;
	int			servers_n;
	std::vector<parseTree*>	serversPtr;
}	servers ;

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
parseTree	*parseFile(char	*file);

//tree funcs - find, search

void		findNode(parseTree *root, parseTree **find_node, std::string find);
parseTree	*findLocation(struct client *client);
bool		findFile(std::string &dirFind, std::string &file);
std::string *getMultiMapValue(std::multimap<std::string, std::string> &map, std::string key);
bool		isInMultiMapKey(std::multimap<std::string, std::string> &map, std::string key);
bool		isInMultiMapValue(std::multimap<std::string, std::string> &map, std::string key, std::string value);

//--HTTP REQUEST---
int		readEvent(struct client *client);
//int		readEvent(clientQueue &Queue, struct kevent *client, struct kevent *client_event, int kq);
int		writeEvent(struct client *client);
//void	writeEvent(bTreeNode *server, clientQueue &Queue, int ident, struct kevent *client_event, int kq);
//HttpRequest loadRequest(char *buffer);
void		loadRequest(HttpRequest *request);
std::string	getRequestedFile(struct client *client, std::vector<std::string> &redirs);
std::string getResponseBody(std::string fileToReturn);
std::string	getStatus(int status);
std::string getResponseHeader(HttpRequest &currentRequest, std::string &body);
//sort-insert funcs
int		binarySearch(std::vector<parseTree*> &vec, parseTree *insert);
void	binaryInsert(std::vector<parseTree *> &vec, parseTree *insert);

//directories-locations
int	cmpDirectories(std::string &s1, std::string &s2);
int	cmpLocations(parseTree *loc, parseTree *cmp);
int	cmpDirectives(void *loc, void *cmp);

//--HTTP REQUEST---
int			readEvent(struct client *client);
int			writeEvent(struct client *client);
void		loadRequest(HttpRequest *request);
std::string	getRequestedFile(parseTree	*server, client *client);
std::string getResponseBody(std::string fileToReturn);
std::string	getStatus(int status);
std::string getResponseHeader(HttpRequest &currentRequest, std::string &body);
std::string GetResponse(parseTree	*server, std::string &url);
void ResponseToMethod(client *client);

//CHUNKED REQUEST
void 	postHeaderChunk(struct client *client, size_t lim);
void	readBodyChunked(struct client *client);

//HTTP METHODS
void	callMultiPart(struct client *client, std::string &path);
void	postMultiPartForm(std::string &route, const char *body, std::string &boundary, size_t size);
void	postText(std::string &route, const char *body, size_t size);
void	postUrlEncoded(std::string &route, const char *body, size_t size);
//LOCATE
size_t locate(const char *haystack, const char *needle, size_t i, size_t size, size_t nsize);

//---SOCKET---
void 	setNonBlocking(int fd);
int		getServerSocket(sockaddr_in *addr, int port);
void	bindAndListen(int sock, sockaddr_in *addr);
int		pollEvents(std::vector<parseTree *> &servers, t_ports *ports);

//---CGI---
std::string getCgi(std::string script);
void		CGIForward(client *client);

//ERRORS
std::string	getStatus(int status);
std::string	getErrorPath(struct client *client, int error);
void	getErrorResponse(struct client *client, int error);

//MULTIMAP
typedef std::multimap<std::string, std::string>::iterator itmap;
typedef std::pair<itmap, itmap> itr;
std::string *getMultiMapValue(std::multimap<std::string, std::string> &map, std::string key);
bool		isInMultiMapKey(std::multimap<std::string, std::string> &map, std::string key);
bool		isInMultiMapValue(std::multimap<std::string, std::string> &map, std::string key, std::string value);
bool 		isInMultiMapValues(std::multimap<std::string, std::string> &map, std::string key, std::string values[], size_t len);
std::string	getMultiMapValueKeys(std::multimap<std::string, std::string> &map, std::string keys[], size_t len);