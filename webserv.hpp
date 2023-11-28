#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/event.h>
#include <vector>
#include <list>
#include "get_next_line.hpp"

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