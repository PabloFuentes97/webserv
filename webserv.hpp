#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
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