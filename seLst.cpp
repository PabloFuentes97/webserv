#include "webserv.hpp"

seNode	*newseNode()
{
	seNode *node = new seNode;
	node->elem_n = 0;
	bzero(node->elem, sizeof(node->elem));
	node->next = NULL;
	return (node);
}

void	seLstPushBack(seLst &lst, seNode *newNode)
{
	if (!lst.head || !newNode)
		return ;
	
	lst.tail->next = newNode;
	lst.tail = newNode;
	lst.size++;

}

void	seLstFree(seLst &lst)
{
	seNode *node = lst.head;
	seNode *aux;

	while (node)
	{
		aux = node;
		node = node->next;
		aux->next = NULL;
		free(aux);
	}
}

char	*seLstToStr(seLst &lst)
{
	char	*str = (char *)malloc(lst.bytes + 1);
	seNode *node = lst.head;
	int	i = 0;
	while (node)
	{
		int	len = node->elem_n;
		for (int j = 0; j < len; i++, j++)
			str[i] = node->elem[j];
		node = node->next;
	}
	str[i] = '\0';
	std::cout << "----------Hizo bien transformación de lista a char*------------" << std::endl;
	std::cout << str << std::endl;
	return (str);
}
