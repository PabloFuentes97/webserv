#include "../../includes/webserv.hpp"

void	findNode(parseTree *root, parseTree **find_node, std::string	find)
//quiza pasar un puntero a una funcion de comparar, que ya compare los valores para determinar si coincide o no
{
	std::cout << "Estoy en contexto: " << root->context._name<< std::endl;

	if (root->context._name == find)
	{
		*find_node = root;
		return ;
	}
	if (!root->childs.empty())
	{
		for (size_t i = 0; i < root->childs.size(); i++)
			findNode(root->childs[i], find_node, find);
	}
}

int	cmpDirectories(std::string &dir1, std::string &dir2)
{
	std::vector<std::string>	subDirs1;
	std::vector<std::string>	subDirs2;
	
	//tokenize dirs
	{
		//dir1
		{
			std::stringstream	dir1Stream(dir1);
			std::string			subDir1;
			std::cout << "SUBDIRS1" << std::endl;
			while (getline(dir1Stream, subDir1, '/'))
			{
				std::cout << subDir1 << std::endl;
				subDirs1.push_back(subDir1);
			}
		}
		//dir2
		{
			std::stringstream	dir2Stream(dir2);
			std::string			subDir2;
			std::cout << "SUBDIRS2" << std::endl;
	 		while (getline(dir2Stream, subDir2, '/'))
			{
				std::cout << subDir2 << std::endl;
				subDirs2.push_back(subDir2);
			}
		}
	}
	//cmp dirs
	int	len = std::min(subDirs1.size(), subDirs2.size());
	for (int i = 0; i < len; i++)
	{
		int dif = strcmp(subDirs1[i].c_str(), subDirs2[i].c_str());
		if (dif != 0)
			return (dif);
	}
	return (0);
}

size_t	skip_char(std::string &s, size_t pos, char c)
{
	for (; pos < s.size() && s[pos] == c; pos++);
	if (s[pos] == '\0')
		return (-1);
	return (pos);
}

size_t	move_to_char(std::string &s, size_t pos, char c)
{
	for (; pos < s.size() && s[pos] != c; pos++);
	if (s[pos] == '\0')
		return (-1);
	return (pos);
}

typedef struct	int_tuple{
	int	i;
	int	e;
} int_tuple;

std::vector<int_tuple>	del_ranges(std::string &s, char del)
{
	std::vector<int_tuple>	ranges;
	int_tuple	range;
	size_t	i = 0;
	size_t	len = s.size();
	while (i < len)
	{
		range.i = skip_char(s, i, del);
		if (range.i < 0)
		{
			std::cout << "No hay siguiente valor de inicio para del" << std::endl;
			break ;
		}
		range.e = move_to_char(s, range.i, del) - 1;
		if (range.e < 0)
		{
			std::cout << "No hay siguiente valor de final para del" << std::endl;
			break ;
		}
		ranges.push_back(range);
		i = range.e + 1;
	}
	return (ranges);
}

int	cmp_str_ranges(std::string &s1, size_t pos1, size_t n1, std::string &s2, size_t pos2, size_t n2)
//probar a pasar un char* ya en la pos correcta, en vez del string entero desde el comienzo con la pos
{
	if (pos1 + n1 >= s1.size())
		return (-2);
	if (pos2 + n2 >= s2.size())
		return (-3);
	if ((n1 - pos1) != (n2 - pos2))
		return (-4);
	int	dif;
	for ( ; pos1 < n1; pos1++)
	{
		dif = s1[pos1] - s2[pos1];
		if (dif != 0)
			return (dif);
	}
	return (0);
}

int	cmp_str_ranges(const char *s1, size_t n1, const char *s2, size_t n2)
{
	if (!s1 || !s2)
		return (-1);
	if (n1 != n2)
		return (-2);
	int	dif;
	for (size_t i = 0 ; i < n1; i++)
	{
		//std::cout << "Comparar: " << s1[i] << " y " << s2[i] << std::endl;
		dif = s1[i] - s2[i];
		if (dif != 0)
			return (dif);
	}
	return (0);
}

int	cmpPaths(std::string &dir1, std::string &dir2)
{
	std::vector<int_tuple>	ranges = del_ranges(dir1, '/');
	if (ranges.empty())
	{
		std::cout << "Ranges1 esta vacío, no hay rangos separables" << std::endl;
	}
	std::cout << "Ranges1: " << std::endl;
	for (size_t i = 0; i < ranges.size(); i++)
	{
		std::cout << "Rango " << i << std::endl;
		std::cout << "Inicio: " << ranges[i].i << " | Final: " << ranges[i].e << std::endl;
	}
	std::vector<int_tuple>	ranges2 = del_ranges(dir2, '/');
	if (ranges2.empty())
	{
		std::cout << "Ranges2 esta vacío, no hay rangos separables" << std::endl;
	}
	std::cout << "Ranges2: " << std::endl;
	for (size_t i = 0; i < ranges.size(); i++)
	{
		std::cout << "Rango " << i << std::endl;
		std::cout << "Inicio: " << ranges[i].i << " | Final: " << ranges[i].e << std::endl;
	}
	int dif = ranges.size() != ranges2.size();
	if (dif != 0)
		return (dif);
	for (size_t i = 0; i < ranges.size(); i++)
	{
		dif = cmp_str_ranges(dir1, ranges[i].i, ranges[i].e - ranges[i].i + 1,
				dir2, ranges2[i].i, ranges2[i].e - ranges2[i].i + 1);
		if (dif != 0)
			return (dif);
	}
	return (0);
}

std::vector<int_tuple>	setRangesDelRev(std::string &s, char del, int pos)
{
	int_tuple				ranges;
	size_t					find;
	std::vector<int_tuple>	rangesVec;

	if (s.size() <= 1)
		return (rangesVec);
	while (pos > 0)
	{
		std::cout << "Iterador de final: " << pos << std::endl;
		//sleep(2);
		ranges.e = pos;
		find = s.rfind(del, pos);
		std::cout << "Find: " << find << std::endl;
		if (find == std::string::npos)
		{
			std::cout << "Deja de encontrar del" << std::endl;
			break ;
		}
		ranges.i = find + 1;
		std::cout << "Iterador de principio: " << ranges.i << std::endl;
		rangesVec.insert(rangesVec.begin(), ranges);
		pos = find - 1;
		//std::cout << "Pos restado: " << pos << std::endl;
		while (pos > 0 && s[pos] == del)
		{
			pos--;
			std::cout << "RESTA A POS: " << pos << std::endl;
		}
		std::cout << "Pos restado: " << pos << std::endl;
	}
	std::cout << "SALGO DE BUCLE" << std::endl;
	for (size_t i = 0; i < rangesVec.size(); i++)
	{
		std::cout << "RANGE: START: " << rangesVec[i].i << " | END: " << rangesVec[i].e << std::endl;
		for (int j = rangesVec[i].i; j <= rangesVec[i].e; j++)
			std::cout << s[j];
		std::cout << std::endl;
		//sleep(3);
	}
	std::cout << "TERMINO RANGO" << std::endl;
	return (rangesVec);
}

int	cmpLocUri(std::string &loc, std::string &url)//, std::string &url
{
	int	i = loc.size() - 1;
	std::cout << "------------HACER RANGOS DE LOC------------" << std::endl;
	if (loc[i] == '/')
	{
		for (; loc[i] != '/' && i >= 0; i--);
	}
	std::vector<int_tuple>	rangesLoc = setRangesDelRev(loc, '/', i);
	if (!rangesLoc.size())
		return (0);
	i = url.size() - 1;
	for (; url[i] != '/' && i >= 0; i--);
	std::cout << "------------HACER RANGOS DE URI------------" << std::endl;
	std::vector<int_tuple>	rangesURI = setRangesDelRev(url, '/', i);
	std::cout << "Size de rangesLoc: " << rangesLoc.size() << std::endl;
	//if (!rangesURI.size() && rangesLoc.size())
	//	return (1);
	if (!rangesURI.size())
		return (1);
	/*
	if (!rangesURI.size())
	{
		int_tuple	tuple = {1, url.size() - 1};
		rangesURI.push_back(tuple);
	}
	else if (!rangesURI.size() && !rangesLoc.size())
		return (0);
	*/
	for (size_t i = 0; i < rangesLoc.size(); i++)
	{
		std::cout << "Comparar LOC: ";
		for (int j = rangesLoc[i].i; j <= rangesLoc[i].e; j++)
			std::cout << loc[j];
		std::cout << " y URI: ";
		for (int j = rangesURI[i].i; j <= rangesURI[i].e; j++)
			std::cout << url[j];
		std::cout << std::endl;
		int	dif = (rangesLoc[i].e - rangesLoc[i].i) - (rangesURI[i].e - rangesURI[i].i);
		if (dif != 0)
		{
			std::cout << "Distinta longitud" << std::endl;
			return (1);
		}
		dif = cmp_str_ranges(&(loc.c_str()[rangesLoc[i].i]), rangesLoc[i].e - rangesLoc[i].i + 1,
				&(url.c_str()[rangesURI[i].i]), rangesURI[i].e - rangesURI[i].i + 1);
		std::cout << "Retorno de cmp: " << dif << std::endl;
		if (dif != 0)
		{
			std::cout << "No coinciden" << std::endl;
			return (1);
		}
	}
	return (0);
}

parseTree	*findLocation(struct client *client)
{
	parseTree	*loc = NULL;
	parseTree	*server = client->server;
	std::cout << "URL a comparar: " << client->request.url << std::endl;
	for (size_t i = 0; i < server->childs.size(); i++)
	{
		loc = server->childs[i];
		std::cout << "Estoy en contexto location: " << loc->context._args[0] << std::endl;
		std::cout << "Len de location: " <<  loc->context._args[0].length() << std::endl;
		if (!cmpLocUri(loc->context._args[0], client->request.url))
		{
			std::cout << "Hace match de la URL: " << loc->context._args[0] << " y " << client->request.url << std::endl;
			return (loc);
		}
	}
	std::cout << "No hizo ningún match" << std::endl;
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
