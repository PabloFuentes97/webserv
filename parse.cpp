#include "webserv.hpp"

size_t	countCharinStr(char *str, char c)
{
	int	count = 0;
	
	for (int i = 0; i < strlen(str); i++)
		if	(str[i] == c)
			count++;
	return (count);
}

size_t	countCharinStr2(std::string str, char c)
{
	int	count = 0;
	
	for (int i = 0; i < str.length(); i++)
		if	(str[i] == c)
			count++;
	return (count);
}

char	*readFileSeLst(int fd)
{
	seLst	lst = {newseNode(), lst.head, 0, 0};
	int		lines = 0;
	seNode 	*node = lst.head;
	ssize_t	readBytes = read(fd, &node->elem, MAX_READ);
	node->elem_n = readBytes;
	lst.bytes = readBytes;
	while (readBytes > 0)
	{
		if (node->elem_n + MAX_READ >= MAX_SE_ELEM)
		{
			node->elem[node->elem_n] = '\0';
			seLstPushBack(lst, newseNode());
			node = lst.tail;
		}
		if (readBytes < 0)
		{
			seLstFree(lst);
			return (NULL);
		}
		lines += countCharinStr(&node->elem[node->elem_n], '\n');
		readBytes = read(fd, &node->elem[node->elem_n], MAX_READ);
		node->elem_n += readBytes;
		lst.bytes += readBytes;
	}
	node->elem[node->elem_n] = '\0';
	return (seLstToStr(lst));
}

bool	validConfigFile(char **matrix)
{

	return (1);
}

typedef struct bTreeNode
{
	std::string					contextName;
	size_t						contextType;
	size_t						contextOperation;
	size_t						contextArgs;
	std::vector<std::string>	directives; //cambiarlo a pair
	std::vector<bTreeNode*>		childs;
} bTreeNode;


typedef struct	s_token
{
	size_t	line;
	size_t	type;

}	t_token;

void	parseContext(bTreeNode *root, std::vector<std::string> &file)
{
	int move = 0;
	bTreeNode	*child;
	std::pair<std::string, std::string> keyVal;
	std::list<bTreeNode *>	nodes; //stack de nodos - se añade cuando entra a un contexto, se borra cuando termina, vuelve al anterior, evitar recursividad
	std::vector<std::string>tokens;
	/*while(1)


	{

		std::cout << "Hola"
			<< std::endl;

	}*/

	nodes.push_front(root);
	while (move < file.size())
	{
		/*
		TOKENIZAR LA LÍNEA: 
		TIPOS DE TOKENS: 
		1) "STRINGS"
		2) '{' (ABRE CONTEXTO)
		3) '}' (CIERRA CONTEXTO)
		4) ';' (CORTE DE LÍNEA)
		5) SÍMBOLOS DE EXPRESIONES REGULARES ('*', '~', '$', etc.)
		6) '#' (COMENTARIOS)
		*/
		//std::cout << "Línea: " << file[move] << std::endl;
		if (countCharinStr2(file[move], '{') > 0)
		{
			//(*context)++;
			//std::cout << "Contexto: " << file[move] << std::endl;
			//std::cout << "Abre contexto: " << file[move] << std::endl;
			child = new bTreeNode();
			child->contextName = file[move];
			root->childs.push_back(child);
			nodes.push_front(child);
			root = nodes.front();
			//std::cout << "Nuevo root: " << root->contextName << std::endl;
			//std::cout << "Reasigna root" << std::endl;
			//parseContext(child, file, move + 1, pointer, context);
			/*(*context)--;
			if ((*context) == 0 && *pointer >= file.size())
			{
				//std::cout << "Contexto principal de nuevo: " << *context << std::endl;
				return ;
			}*/
			//std::cout << "Vuelvo a contexto: " << *context << std::endl;
			//break ;
			//move = *pointer;
		}
		else if (countCharinStr2(file[move], '}') > 0)
		{
			//std::cout << "Cierra contexto: " << file[move] << std::endl;
			//*pointer = move + 1;
			nodes.pop_front();
			//if (nodes.empty())
			//	return ;
			root = nodes.front();
		}
		else
		{
			std::pair<std::string, std::string>	keyVal;
			int i;
			int	j;
			for (i = 0; i < file[move].size() && (file[move][i] == ' ' || file[move][i] == '\t'); i++);
			//std::cout << "Después de espacios: " << &(file[move][i]) << std::endl;
			for (j = i + 1; j < file[move].size() && file[move][j] != ' '; j++);
			keyVal.first = file[move].substr(i, j - i + 1);
			//std::cout << "Key: " << keyVal.first << std::endl;
			for (i = j + 1; i < file[move].size() && file[move][i] == ' '; i++);
			for (j = file[move].size(); j > 0 && file[move][j] == ' '; j--);
			keyVal.second = file[move].substr(i, j - i + 1);
			//std::cout << "Value: " << keyVal.second << std::endl;
			root->directives.push_back(file[move]);	
		}
		move++;
		//sleep(1);
	}
}

void	printBTree(bTreeNode *root)
{
	std::cout << "Context: " << root->contextName << std::endl;
	std::cout << "Directivas: " << std::endl;
	for (int i = 0; i < root->directives.size(); i++)
		std::cout << root->directives[i];
	std::cout << std::endl;
	for (int i = 0; i < root->childs.size(); i++)
	{
		std::cout << "Hijo " << i << " de: " << root->contextName << std::endl;
		printBTree(root->childs[i]);
	}
		
}

void	parseFile(int fd)
{
	std::vector<std::string>	lines;

	char	*line = get_next_line(fd);

	while (line)
	{
		if (!(line[0] == '\n'))
			lines.push_back(line);
		free(line);
		line = get_next_line(fd);
	}
	for (int i = 0; i < lines.size(); i++)
		std::cout << lines[i];
	std::cout << std::endl;
	bTreeNode *root = new bTreeNode();
	root->contextName = "main";
	parseContext(root, lines);
	printBTree(root);
}
