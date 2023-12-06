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

int	main(void)
{
	std::string	dir = "/Users/pfuentes/ejercicios/CPP/webserver_git";
	std::string	file = "webserv";
	if (findFile(dir, file))
		std::cout << "Encontró el fichero!" << std::endl;
	else
		std::cout << "No lo encontró..." << std::endl;
	std::string	URI = "/kapouet/pouic/toto/pouet";
	//std::string	URI = "/";
	//std::string	URI = "/pepe/manuel/";
	Location	loc;
	parseURI(loc, URI);
	return (0);
}