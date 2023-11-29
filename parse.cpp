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


typedef struct bTreeNode
{
	std::string					contextName;
	size_t						contextType;
	size_t						contextOperation;
	size_t						contextArgs;
	//cambiarlo contenido a <pair>: key-value, pero valores puede ser una lista, múltiples valores
	std::vector<std::string>	keys;
	std::vector<std::string>	values;
	std::vector<int>	mapKeyValues;
	std::map<std::string, std::string>	directivesMap;
	std::vector<std::pair<std::string, std::vector<std::string> > >	directives;
	std::vector<bTreeNode*>		childs;
} bTreeNode;

//más "orientado a objetos" cada token un objeto con sus atributos "individuales"
typedef struct	s_token
{
	size_t		line;
	size_t		type;
	std::string	value;
}	t_token;

//más "orientado a datos" una estructura que guarda en arrays las propiedades de todos los tokens
typedef struct s_tokens
{
	int		n;
	size_t	*line;
	size_t	*type;
	std::vector<std::string>	values;
}	t_tokens;


void	tokenizeFile(char *file, std::vector<t_token> &tokens, std::string &del)
{
	std::fstream	readTokens(file);
	
	std::string	tokenStr;
	t_token	token;
	int	start;
	int	end;
	int	flag;
	while (!readTokens.eof())
	{	
		readTokens >> tokenStr;
		start = 0;
		end = 0;
		flag = 0;
		for (int i = 0; i < tokenStr.length(); i++)
		{
			for (int j = 0; j < del.length(); j++)
			{
				if (tokenStr[i] == del[j])
				{
					end = i;
					std::cout << "Encontró: " << del[j] << std::endl;
					std::cout << "Start: " << tokenStr[start] << " , End: " << tokenStr[end] << std::endl;
					//si lo anterior no es un símbolo especial, que ya está añadido, sino una palabra "normal"
					if (start != end)
					{
						token.value = tokenStr.substr(start, end - start);
						tokens.push_back(token);
					}
					token.value = del[j];
					tokens.push_back(token);
					end++;
					start = end;
					flag = 1;
					break ;
				}
			}
		}
		if (flag == 0)
		{
			token.value = tokenStr;
			tokens.push_back(token);
		}
	}
}

bool	validTokensConfig(std::vector<t_token> &tokens)
{

	return (1);
}

/*void	parseContextLines(bTreeNode *root, std::vector<std::string> &file)
{
	int move = 0;
	bTreeNode	*child;
	//std::pair<std::string, std::string> keyVal;
	std::list<bTreeNode *>	nodes; //stack de nodos - se añade cuando entra a un contexto, se borra cuando termina, vuelve al anterior, evitar recursividad
	nodes.push_front(root);
	while (move < file.size())
	{
		if (file[move].empty())
			continue ;
		if (countCharinStr2(file[move], '{') > 0)
		{
			child = new bTreeNode();
			child->contextName = file[move];
			root->childs.push_back(child);
			nodes.push_front(child);
			root = nodes.front();
		}
		else if (countCharinStr2(file[move], '}') > 0)
		{
			nodes.pop_front();
			root = nodes.front();
		}
		else
			root->directives.push_back(file[move]);	
		move++;
	}
}*/

void	parseContextTokens(bTreeNode *root, std::vector<t_token> &tokens)
{
	bTreeNode	*child;
	//std::pair<std::string, std::string> keyVal;
	std::list<bTreeNode *>	nodes; //stack de nodos - se añade cuando entra a un contexto, se borra cuando termina, vuelve al anterior, evitar recursividad
	nodes.push_front(root);
	int	initDirective = 0;
	int	endDirective = 0;
	for (int i = 0;  i < tokens.size(); i++)
	{
		if (tokens[i].value == "{") //tiene que abrir contexto
		{
			child = new bTreeNode();
			child->contextName = tokens[i - 1].value;
			root->childs.push_back(child);
			nodes.push_front(child);
			root = nodes.front();
			initDirective = i + 1;
		}
		else if (tokens[i].value == "}") //tiene que cerrar contexto
		{
			nodes.pop_front();
			root = nodes.front();
			initDirective = i + 1;
		}
		else if (tokens[i].value == ";") // final de directiva, añadir a lista
		{
			endDirective = i;
			std::cout << "Primer token de directiva: " << tokens[initDirective].value << std::endl;
			root->keys.push_back(tokens[initDirective].value);
			std::cout << "Último token de directiva: " << tokens[endDirective - 1].value << std::endl;
			int	keysNum = root->keys.size() - 1;
			for (int j = initDirective + 1; j < endDirective; j++)
			{
				std::cout << "Value a añadir de directiva: " << tokens[j].value << std::endl;
				root->values.push_back(tokens[j].value);
				root->mapKeyValues.push_back(keysNum);
			}
			initDirective = i + 1;
		}
	}
}

void	printBTree(bTreeNode *root)
{
	std::cout << "Context: " << root->contextName << std::endl;
	std::cout << "Directivas: " << std::endl;
	int j = 0;
	for (int i = 0; i < root->keys.size(); i++)
	{
		std::cout << "Key: " << root->keys[i] << std::endl;
		std::cout << "Values: {";
		while (j < root->mapKeyValues.size() && root->mapKeyValues[j] == i)
		{
			std::cout << root->values[j];
			if (j < root->mapKeyValues.size() - 1 && root->mapKeyValues[j + 1] == i)
				std::cout << ", ";
			j++;
		}
		std::cout << "}" << std::endl;
	}
	std::cout << std::endl;
	for (int i = 0; i < root->childs.size(); i++)
	{
		std::cout << "Hijo " << i << " de: " << root->contextName << std::endl;
		printBTree(root->childs[i]);
	}
}

void	parseFile(char	*file)
{
	std::ifstream	readFile(file);
	std::vector<std::string>	lines;

	std::string line;

	while (getline(readFile, line, '\n'))
	{
		//if (!(line[0] == '\n'))
		if (!line.empty())
			lines.push_back(line);
	}

	std::string	del = "{};=";
	std::vector<t_token>	tokens;
	tokenizeFile(file, tokens, del);
	for (int i = 0; i < tokens.size(); i++)
		std::cout << "Token: " << tokens[i].value << std::endl;
	bTreeNode *root = new bTreeNode();
	root->contextName = "main";
	//parseContextLines(root, lines);
	parseContextTokens(root, tokens);
	printBTree(root);
}

int	main(int argc, char **argv)
{
	if (argc != 2)
		return (1);
	
	parseFile(argv[1]);
	return (0);
}