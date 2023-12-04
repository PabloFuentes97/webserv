#include "webserv.hpp"

bool	tokenizeFile(char *file, std::vector<t_token> &tokens, std::string &del) //comprobar también aquí que lo anterior a un delimitador es un str
{
	std::fstream	readTokens(file);
	
	if (readTokens.fail())
	{
		std::cout << "Fichero erróneo" << std::endl;
		return (false);
	}
	std::string	tokenStr;
	t_token	token;
	int	start;
	int	end;
	int	flag;
	int	brackets = 0;
	while (readTokens >> tokenStr) //!readTokens.eof()
	{	
		//readTokens >> tokenStr;
		start = 0;
		end = 0;
		flag = 0;
		std::cout << "Tokenstr: " << tokenStr << std::endl;
		for (int i = 0; i < tokenStr.length(); i++)
		{
			for (int j = 0; j < del.length(); j++)
			{
				if (tokenStr[i] == del[j])
				{
					if (del[j] == '{')
					{
						std::cout << "Abro bracket" << std::endl;
						brackets++;
					}
					else if (del[j] == '}')
					{
						std::cout << "Cierro bracket" << std::endl;
						brackets--;
					}
					if (brackets < 0)
					{
						std::cout << "Número y orden de brackets es erróneo" << std::endl;
						return (false);
					}
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
	std::cout << "Diferencia entre brackets de apertura y cierre: " << brackets << std::endl;
	if (brackets != 0)
	{
		std::cout << "Al final, número y orden de brackets es erróneo" << std::endl;
		return (false);
	}
	return (true);
}

size_t	matrixLenChar(char const **matrix)
{
	size_t i;

	for (i = 0; matrix[i] != NULL; i++);
	return (i);
}

/*bool	validTokensSyntax(std::vector<t_token> &tokens)
{
	char const	*contextNames[] = {"main", "http", "server", "location", NULL};
	char const	*mainKeys[] = {"workers", NULL};
	char const	*eventsKeys[] = {"eee", NULL}; //main
	char const	*httpKeys[] = {"include", NULL}; //main

	char const	*serverKeys[] = {"listen", "server_name", "error_page", "root", "index", NULL};
	char const	*locationKeys[] = {"root", "alias", "try_files", NULL};
	char const	**contextPointers[] = {mainKeys, httpKeys, serverKeys, locationKeys, NULL}; //punteros a las matrices de keys de cada contexto
	int			mapContextLevels[] = {0, 1, 1, 2, 3};
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
}*/

//cambiarlo a char ** para alocar la memoria exacta, no tener que realocar constantemente
std::vector<std::string>	validContexts(std::string &context)
{
	std::vector<std::string>	contexts;
	if (context == "main")
	{
		contexts.push_back("events");
		contexts.push_back("http");
	}
	else if (context == "http")
		contexts.push_back("server");
	else if (context == "server")
		contexts.push_back("location");
	return (contexts);
}

std::vector<std::string>	validDirectives(std::string	&context)
{
	std::vector<std::string>	directives;
	
	if (context == "main")
		directives.push_back("workers");
	else if (context == "events")
		directives.push_back("prueba");
	else if (context == "http")
		directives.push_back("hola");
	else if (context == "server")
	{
		directives.push_back("listen");
		directives.push_back("server_name");
		directives.push_back("error_page");
		directives.push_back("root");
		directives.push_back("index");
	}
	else if (context == "location")
	{
		directives.push_back("root");
		directives.push_back("alias");
		directives.push_back("try_files");	
	}
	return (directives);
}



bool	strInVector(std::string &str, std::vector<std::string> &vector) //usar para comparar tanto directivas como contextos
//hacer que en vez de devolver un bool devuelva un puntero, si no está es NULL, si está puedo acceder a él directamente
{
	std::cout << "Buscar si str: " << str << " está en vector: " << std::endl; 
	for (int i = 0; i < vector.size(); i++)
	{
		std::cout << "A comparar: " << vector[i] << std::endl;
		if (str == vector[i])
			return (true);
	}
	return (false);
}

bTreeNode	*copyBTreeNode(bTreeNode *node)
{
	bTreeNode	*copy = new bTreeNode();
	copy->contextName = node->contextName;
	return (copy);
}

bool	parseContextTokens(bTreeNode *root, bTreeNode *check, std::vector<t_token> &tokens)
{
	bTreeNode	*child; //nodo actual del árbol donde guardar
	bTreeNode	*checkChild; //nodo del árbol a crear

	std::list<bTreeNode *>	nodes; //stack de nodos - se añade cuando entra a un contexto, se borra cuando termina, vuelve al anterior, evitar recursividad
	std::list<bTreeNode *>	checkNodes; //stack de nodos del árbol para checkear

	nodes.push_front(root);
	checkNodes.push_front(check);
	int	initDirective = 0;
	int	endDirective = 0;
	for (int i = 0;  i < tokens.size(); i++)
	{
			
		if (tokens[i].value == "{") //tiene que abrir contexto y saltar al siguiente
		{
			if (initDirective == i)
			{
				std::cout << "Token anterior es un delimitador" << std::endl;
				return (false);
			}
			std::cout << "-----------------Detecto nuevo contexto a abrir: " << tokens[initDirective].value << std::endl;
			std::cout << "Estoy en contexto: " << root->contextName << std::endl;
			std::cout << "Estoy en contexto de checkeo: " << check->contextName << std::endl;
			if (!strInVector(tokens[initDirective].value, check->allSubContexts)) //compruebo que el contexto está dentro de los válidos
			{
				std::cout << "Contexto pasado no está permitido en contexto actual" << std::endl;
				//liberar árbol auxiliar -> crear función para liberar árbol
				return (false);
			}
			std::cout << "Contexto sí es válido" << std::endl;
			//está bien, creo nuevo hijo
			child = new bTreeNode();
			child->contextName = tokens[initDirective].value;
			for (int start = initDirective + 1; start < i; start++) //añade argumentos del contexto si los hay
				child->contextArgs.push_back(tokens[start].value);
			//añadir hijo a root actual, y cambiar root actual al nuevo hijo para movernos por él
			root->childs.push_back(child);
			root->childsNames.push_back(child->contextName);
			nodes.push_front(child);
			root = child;
			std::cout << "Asigno bien contexto hijo" << std::endl;
			if (!strInVector(child->contextName, check->childsNames)) //checkeo si este tipo de contexto ya está en el nodo de checkeo
			//si no está lo añado, si está no, para evitar redundancia
			{
				std::cout << "Contexto actual no está en lista de subcontextos de checkeo" << std::endl;
				checkChild = copyBTreeNode(child);
				check->childs.push_back(checkChild);
				check->childsNames.push_back(checkChild->contextName);
				checkNodes.push_front(checkChild);
				check = checkChild;
				check->allSubContexts = validContexts(check->contextName);
				check->keys = validDirectives(check->contextName);
			}
			else //checkeo que tipo de contexto es para moverme a él, al puntero del hijo
			{
				std::cout << "Contexto actual sí está en lista de subcontextos de checkeo" << std::endl;
				for (int i = 0; i < check->childsNames.size(); i++)
				{
					if (root->contextName == check->childsNames[i])
					{
						checkNodes.push_front(check->childs[i]);
						check = check->childs[i];
						break ;
					}
				}
			}
			std::cout << "Entro en nuevo contexto: " << std::endl;
			std::cout << "Estoy en contexto: " << root->contextName << std::endl;
			std::cout << "Estoy en contexto de checkeo: " << check->contextName << std::endl;
			initDirective = i + 1;
		}
		else if (tokens[i].value == "}") //tiene que cerrar contexto y volver al anterior
		{
			std::cout << "--------------------Cierro contexto: " << std::endl;
			std::cout << "Contexto a cerrar: " << root->contextName << std::endl;
			std::cout << "Contexto de checkeo a cerrar: " << check->contextName << std::endl;
			nodes.pop_front();
			root = nodes.front();
			checkNodes.pop_front();
			check = checkNodes.front();
			initDirective = i + 1;
			std::cout << "Estoy en contexto: " << root->contextName << std::endl;
			std::cout << "Estoy en contexto de checkeo: " << check->contextName << std::endl;
		}
		else if (tokens[i].value == ";") // final de directiva, añadir a lista
		{
			if (initDirective == i)
			{
				std::cout << "Token anterior es un delimitador" << std::endl;
				return (false);
			}
			std::cout << "Checkear si directiva es válida: " << tokens[initDirective].value << std::endl;
			endDirective = i;
			if (!strInVector(tokens[initDirective].value, check->keys))
			{
				//liberar árbol auxiliar -> crear función para liberar árbol
				std::cout << "Key no es válida" << std::endl;
				return (false);
			}
			std::cout << "Key es válida" << std::endl;
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
	return (true);
}

void	printBTree(bTreeNode *root, int type)
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
	if (type == 0)
	{
		std::cout << "Directivas posibles: " << std::endl;
		int j = 0;
		for (int i = 0; i < root->keys.size(); i++)
		{
			std::cout << root->keys[i] << ", ";
		}
		std::cout << std::endl;
	}
	else
	{
		std::cout << "Directivas: " << std::endl;
		int j = 0;
		for (int i = 0; i < root->keys.size(); i++)
		{
			std::cout << "Key: " << root->keys[i] << ", ";
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
	}
	for (int i = 0; i < root->childs.size(); i++)
	{
		std::cout << "Hijo " << i << " de: " << root->contextName << std::endl;
		printBTree(root->childs[i], type);
	}
}

bool	parseFile(char	*file)
{
	std::string	del = "{};=";
	std::vector<t_token>	tokens;
	if (!tokenizeFile(file, tokens, del))
	{
		std::cout << "Error al tokenizar" << std::endl;
		return (false);
	}
	for (int i = 0; i < tokens.size(); i++)
		std::cout << "Token: " << tokens[i].value << std::endl;
	bTreeNode	*root = new bTreeNode();
	bTreeNode	*check = new bTreeNode();
	root->contextName = "main";
	check->contextName = "main";
	check->allSubContexts = validContexts(check->contextName);
	check->keys = validDirectives(check->contextName);
	//if (!validTokensSyntax(tokens))
	//	return (false);
	if (!parseContextTokens(root, check, tokens))
		return (false);
	std::cout << "---------------Imprimir árbol de checkeo---------------" << std::endl;
	printBTree(check, 0);
	std::cout << "---------------Imprimir árbol de fichero de configuración---------------" << std::endl;
	printBTree(root, 1);
	return (true);
}

int	main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "Error: bad number of arguments" << std::endl;
		return (1);
	}
	union context_types prueba;
	std::cout << "Sizeof context_types: " << sizeof(prueba) << std::endl;

	if (!parseFile(argv[1]))
	{
		std::cout << "Hubo un error al parsear el fichero" << std::endl;
		return (1);
	}
	return (0);
}