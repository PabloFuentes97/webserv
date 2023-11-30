#include "webserv.hpp"

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

size_t	matrixLenChar(char const **matrix)
{
	size_t i;

	for (i = 0; matrix[i] != NULL; i++);
	return (i);
}

void	fullValidTree(bTreeNode &root)
{
	char const	*contextNames[] = {"main", "http", "server", "location", NULL};
	char const	*httpKeys[] = {"include", NULL};
	char const	*mainKeys[] = {"workers", NULL};
	char const	*serverKeys[] = {"listen", "server_name", "error_page", "root", "index", NULL};
	char const	*locationKeys[] = {"root", "alias", "try_files", NULL};
	
}

bool	validTokensSyntax(std::vector<t_token> &tokens)
{
	char const	*contextNames[] = {"main", "http", "server", "location", NULL};
	char const	*httpKeys[] = {"include", NULL};
	char const	*mainKeys[] = {"workers", NULL};
	char const	*serverKeys[] = {"listen", "server_name", "error_page", "root", "index", NULL};
	char const	*locationKeys[] = {"root", "alias", "try_files", NULL};
	char const	**contextPointers[] = {mainKeys, httpKeys, serverKeys, locationKeys, NULL}; //punteros a las matrices de keys de cada contexto

	int			initDirective = 0;
	int			endDirective = 0;
	int			validContext;
	std::string	currContext = "main";
	int			currContextIndex = 0;
	std::list<std::string>	contextsVisited;
	char	**checkContext;

	std::cout << "------------------Checkear sintaxis de tokens------------------" << std::endl << std::endl;
	contextsVisited.push_front(currContext);
	for (int i = 0;  i < tokens.size(); i++)
	{
		
		if (tokens[i].value == "{") //tiene que abrir contexto y saltar al siguiente
		{
			validContext = 0;
			currContext = tokens[initDirective].value; //saber en qué contexto estoy
			std::cout << "Entro en contexto: " << currContext << std::endl;
			currContextIndex++;
			contextsVisited.push_front(currContext);
			for (int j = 0; j < matrixLenChar(contextNames); j++) //recorrer lista de posibles nombres de contexto, comparar si coincide con contexto actual
			{
				std::cout << "Compara contexto actual: " << currContext << " con: " << contextNames[j] << std::endl;
				if (!currContext.compare(contextNames[j]))
				{
					validContext = 1;
					break ;
				}
			}
			if (validContext == 0)
			{
				std::cout << "No coincide el nombre del contexto con uno de los posibles" << std::endl;
				return (false);
			}
			initDirective = i + 1;
		}
		else if (tokens[i].value == "}") //tiene que cerrar contexto y volver al anterior
		{
			std::cout << "Cierro contexto: " << currContext << std::endl;
			contextsVisited.pop_front();
			currContext = contextsVisited.front();
			currContextIndex--;
			initDirective = i + 1;
		}
		else if (tokens[i].value == ";") // final de directiva, añadir a lista
		{
			endDirective = i;
			int	validKey = 0;
			char const	**currContextPtr = contextPointers[currContextIndex];
			for (int j = 0; j < matrixLenChar(currContextPtr); j++)
			{
				std::cout << "Checkear key: " << tokens[initDirective].value << " con: " << currContextPtr[j] << std::endl;
				if (!tokens[initDirective].value.compare(currContextPtr[j]))
				{
					std::cout << "Coinciden: " << tokens[initDirective].value << " y " << currContextPtr[j] << std::endl;
					validKey = 1;
					break ;
				}
			}
			if (validKey == 0)
			{
				std::cout << "No coincide la key con una de los posibles" << std::endl;
				return (false);
			}
			initDirective = i + 1;
		}
	}
	std::cout << "Sintaxis válida" << std::endl;
	return (true);
}

void	parseContextTokens(bTreeNode *root, std::vector<t_token> &tokens)
{
	bTreeNode	*child;
	std::list<bTreeNode *>	nodes; //stack de nodos - se añade cuando entra a un contexto, se borra cuando termina, vuelve al anterior, evitar recursividad
	nodes.push_front(root);
	int	initDirective = 0;
	int	endDirective = 0;
	for (int i = 0;  i < tokens.size(); i++)
	{
		if (tokens[i].value == "{") //tiene que abrir contexto y saltar al siguiente
		{
			child = new bTreeNode();
			child->contextName = tokens[initDirective].value;
			for (int start = initDirective + 1; start < i; start++) //añade argumentos del contexto si los hay
				child->contextArgs.push_back(tokens[start].value);
			root->childs.push_back(child);
			nodes.push_front(child);
			root = nodes.front();
			initDirective = i + 1;
		}
		else if (tokens[i].value == "}") //tiene que cerrar contexto y volver al anterior
		{
			nodes.pop_front();
			root = nodes.front();
			initDirective = i + 1;
		}
		else if (tokens[i].value == ";") // final de directiva, añadir a lista
		{
			endDirective = i;
			root->keys.push_back(tokens[initDirective].value); //añade key
			int	keysNum = root->keys.size() - 1;
			for (int j = initDirective + 1; j < endDirective; j++) //añade values
			{
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
	if (root->contextArgs.size() > 0)
	{
		std::cout << "Argumentos del contexto: {";
		for (int i = 0; i < root->contextArgs.size(); i++)
		{
			std::cout << root->contextArgs[i];
			if (i < root->contextArgs.size() - 1)
				std::cout << ", ";
		}
		std::cout << "}" << std::endl;
	}
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

bool	parseFile(char	*file)
{
	std::string	del = "{};=";
	std::vector<t_token>	tokens;
	tokenizeFile(file, tokens, del);
	for (int i = 0; i < tokens.size(); i++)
		std::cout << "Token: " << tokens[i].value << std::endl;
	bTreeNode *root = new bTreeNode();
	root->contextName = "main";
	//if (!validTokensSyntax(tokens))
	//	return (false);
	parseContextTokens(root, tokens);
	printBTree(root);
	return (true);
}

int	main(int argc, char **argv)
{
	if (argc != 2)
		return (1);
	
	if (!parseFile(argv[1]))
	{
		std::cout << "Hubo un error al parsear el fichero" << std::endl;
		return (1);
	}
	return (0);
}