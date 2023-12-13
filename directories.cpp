#include "webserv.hpp"

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

bTreeNode	*findLocation(bTreeNode *server, std::string	&location)
{
	bTreeNode	*loc = NULL;

	for (int i = 0; i < server->childs.size(); i++)
	{
		loc = server->childs[i];
		std::cout << "Estoy en contexto location: " << loc->contextArgs[0] << std::endl;
		if (loc->contextArgs[0] == location)
		{
			std::cout << "Encontró location: " << loc->contextArgs[0] << std::endl;
			return (loc);
		}
	}
	std::cout << "No encontró location" << std::endl;
	return (NULL);
}

bool	findFile(std::string &dirFind, std::string &file)
{
	DIR *dir = opendir(dirFind.c_str());
	dirent	*dir_elem = readdir(dir);
	bool	found = false;

	while (dir_elem)
	{
		printf("Elemento de tipo: %d, y nombre: %s\n", dir_elem->d_type, dir_elem->d_name);
		//std::cout << "Elemento de tipo: " << dir_elem->d_type << " y nombre: " << dir_elem->d_name << std::endl;
		if (dir_elem->d_type == 8 && dir_elem->d_name == file)
		{
			std::cout << "Coincide el fichero: " << dir_elem->d_name << std::endl;
			found = true;
			break ;
		}
		dir_elem = readdir(dir);
	}
	closedir(dir);
	return (found);
}

bool	SuccesiveChar(const char *str, char c)
{
	for (int i = 1; i < strlen(str); i++)
	{
		if (str[i] == str[i - 1])
			return (true);
	}
	return (false);
}

size_t	countCharinStr(const char *str, char c)
{
	int	count = 0;
	
	for (int i = 0; i < strlen(str); i++)
		if	(str[i] == c)
			count++;
	return (count);
}

bool	validLocationModifiers(std::string &modifier)
{
	const char	*modifiers[] = {"=", "~", "~*", "^~"}; //= | ~ | ~* | ^~ 
	for (int i = 0; i < 4; i++)
		if (modifiers[i] == modifier)
			return (true);
	return (false);
}

typedef	struct	Location{
	size_t						uriType;
	int							modifier;
	std::vector<std::string>	uriTokens;
	std::vector<std::pair<std::string, std::vector<std::string> > >	directives; 	
}	Location;


/*int	cmpLocations(void *loc, void *cmp) //loc es la location a comparar, cmp es el url
{
	context	*loc_cast = (context *)loc;
	std::string	*cmp_cast = (std::string *)cmp;
	std::string	s1;
	std::string	s2;
	std::cout << "CMP1: " << loc_cast->_args[0] << ", CMP2: " << *cmp_cast << std::endl;

	if (loc_cast->_args[0] == "/")
	{
		std::cout << "Location es directorio raíz" << std::endl;
		return (1);
	}		
	for (int start1 = 1, start2 = 1, i = 1, j = 1; start1 < loc_cast->_args[0].length() && start2 < (*cmp_cast).length(); start1++, start2++)
	{
		//std::cout << "Loc: " << loc_cast->_args[0][i] << std::endl;
		//std::cout << "Cmp: " << (*cmp_cast)[i] << std::endl;
		for ( ; loc_cast->_args[0][start1] && loc_cast->_args[0][start1] != '/'; start1++);
		for ( ; (*cmp_cast)[start2] && (*cmp_cast)[start2] != '/'; start2++);
		s1 = loc_cast->_args[0].substr(i, start1 - i);
		s2 = (*cmp_cast).substr(j, start2 - j);
		std::cout << "S1: " << s1 << ", S2: " << s2 << std::endl;
		if (s1 != s2)
		{
			std::cout << "No coinciden" << std::endl;
			return (0);
		}
		i = start1 + 1;
		j = start2 + 1;
	}
	//if (!loc_cast->_name.compare(loc_cast->_name.length(), 0, *cmp_cast))
	//	return (true);
	return (1);
}*/

void	parseURI(Location &loc, std::string &URI)
{
	std::stringstream			str(URI);
	std::string					token;
	int							slash;

	if (SuccesiveChar(URI.c_str(), '/'))
	{
		std::cout << "Error: Slashes sucesivos" << std::endl;
		return ;
	}
	slash  = countCharinStr(URI.c_str(), '/');
	std::cout << "Número de slashes: " << slash << std::endl;
	if (URI[URI.length() - 1] == '/')
	{
		loc.uriType = 0;
		std::cout << "Es un directorio" << std::endl;
	}
	else
	{
		loc.uriType = 1;
		std::cout << "Es un fichero" << std::endl;
	}
	while (getline(str, token, '/'))
	{
		if (!token.empty())
		{
			std::cout << "Token: " << token << std::endl;
			loc.uriTokens.push_back(token);
		}		
	}
}

/*int	main(void)
{
	std::string	dir = "/Users/pfuentes/ejercicios/CPP/webserv_git";
	std::string	file = "webserv";
	if (findFile(dir, file))
		std::cout << "Encontró el fichero!" << std::endl;
	else
		std::cout << "No lo encontró..." << std::endl;
	std::string	URI = "/kapouet/pouic/toto/pouet";
	std::string	URI4 = "/kapouet/pouic/manuel/pouet";
	//std::string	URI = "/";
	std::string	URI2 = "/pepe/manuel/";
	std::string	URI3 = "/";
	context	c;
	c._args.push_back(URI);
	cmpLocations(&c, &URI4);
	return (0);
}*/