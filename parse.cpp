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
		start = 0;
		end = 0;
		flag = 0;
		for (int i = 0; i < tokenStr.length(); i++)
		{
			for (int j = 0; j < del.length(); j++) //quiza hacer una funcion aparte que busque un char en un str, como un "alfabeto"
			{
				if (tokenStr[i] == del[j])
				{
					if (del[j] == '{')
						brackets++;
					else if (del[j] == '}')
						brackets--;
					if (brackets < 0)
						return (false);
					end = i;
					//si lo anterior no es un símbolo especial, que ya está añadido, sino una palabra "normal"
					if (start != end)
					{
						token.value = tokenStr.substr(start, end - start);
						token.type = 0;
						tokens.push_back(token);
					}
					token.value = del[j];
					token.type = j + 1;
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
	if (brackets != 0)
		return (false);
	return (true);
}

size_t	matrixLenChar(char const **matrix)
{
	size_t i;

	for (i = 0; matrix[i] != NULL; i++);
	return (i);
}

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
			if (!strInVector(tokens[initDirective].value, check->allSubContexts)) //compruebo que el contexto está dentro de los válidos
			{
				std::cout << "Contexto pasado no está permitido en contexto actual" << std::endl;
				//liberar árbol auxiliar -> crear función para liberar árbol
				return (false);
			}
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
			//parte de árbol extra para checkear
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
			initDirective = i + 1;
		}
		else if (tokens[i].value == "}") //tiene que cerrar contexto y volver al anterior
		{
			nodes.pop_front();
			root = nodes.front();
			checkNodes.pop_front();
			check = checkNodes.front();
			initDirective = i + 1;
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
			root->keys.push_back(tokens[initDirective].value); //añade key
			int	keysNum = root->keys.size() - 1; //cambiar esto, que guarde un pair, borrar array para mapear values a key
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

void	findNode(bTreeNode *root, bTreeNode **find_node, std::string	find)
{
	std::cout << "Estoy en contexto: " << root->contextName << std::endl;

	if (root->contextName == find)
	{
		*find_node = root;
		return ;
	}
	if (!root->childs.empty())
	{
		for (int i = 0; i < root->childs.size(); i++)
			findNode(root->childs[i], find_node, find);
	}
}

bool	findLocation(bTreeNode *server, std::string	&location)
{
	bTreeNode	*loc;

	for (int i = 0; i < server->childs.size(); i++)
	{
		loc = server->childs[i];
		std::cout << "Estoy en contexto location: " << loc->contextArgs[0] << std::endl;
		if (loc->contextArgs[0] == location)
		{
			std::cout << "Encontró location: " << loc->contextArgs[0] << std::endl;
			return (true);
		}
	}
	std::cout << "No encontró location" << std::endl;
	return (false);
}

int	pruebaMapTreeServers(bTreeNode *root)
{
	servers	prueba;

	prueba.servers_n = 2;
	//servidor
	int	fd_num = 6;
	int	servers_num = 0;
	int	clients_num = 0;
	for (int i = 0; i < prueba.servers_n; i++, servers_num++, fd_num += 3)
	{
		for (int j = fd_num; j < fd_num + 3; j++, clients_num++)
			prueba.clients.addClient(j, i);
	}
	prueba.serversPtr = root->childs;
	srand(time(NULL));
	int	random;
	for (int i = 0; i < prueba.clients.clientArray.size(); i++)
	{
		random = rand() % 3;
		switch(random)
		{
			case 0: { prueba.clients.clientArray[i].request.url = "/juan"; break; };
			case 1: { prueba.clients.clientArray[i].request.url = "/manuel"; break; };
			case 2: { prueba.clients.clientArray[i].request.url = "/jose"; break; };
		}
	}
	for (int i = 0; i < prueba.clients.clientArray.size(); i++)
	{
		std::cout << "Cliente: " << std::endl; 
		std::cout << "Servidor: " << prueba.clients.clientArray[i].serverID << std::endl;
		std::cout << "Socket de conexión: " << prueba.clients.clientArray[i].fd << std::endl;
		std::cout << "URL: " << prueba.clients.clientArray[i].request.url << std::endl;
		findLocation(prueba.serversPtr[prueba.clients.clientArray[i].serverID], prueba.clients.clientArray[i].request.url);		
	}
	return (0);
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
	bTreeNode	*find = NULL; //http como root
	std::cout << "---------------BUSCAR NODO EN ÁRBOL-------------" << std::endl;
	findNode(root, &find, "http");
	if (find)
		std::cout << "Lo encontró: " << find->contextName << std::endl;
	else
		std::cout << "No lo encontró " << std::endl;
	pruebaMapTreeServers(find);
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