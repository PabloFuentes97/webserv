#include "../includes/webserv.hpp"

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

int	cmpLocUrl(std::string &loc, std::string &url)
{
	std::vector<int_tuple>	rangesLoc = del_ranges(loc, '/');
	if (rangesLoc.empty())
	{
		std::cout << "Ranges de loc esta vacío, no hay rangos separables" << std::endl;
	}
	std::cout << "Ranges de loc: " << std::endl;
	for (size_t i = 0; i < rangesLoc.size(); i++)
	{
		std::cout << "Rango " << i << std::endl;
		std::cout << "Inicio: " << rangesLoc[i].i << " | Final: " << rangesLoc[i].e << std::endl;
	}
	std::vector<int_tuple>	rangesURL = del_ranges(url, '/');
	if (rangesURL.empty())
	{
		std::cout << "rangesURL esta vacío, no hay rangos separables" << std::endl;
	}
	std::cout << "rangesURL: " << std::endl;
	for (size_t i = 0; i < rangesURL.size(); i++)
	{
		std::cout << "Rango " << i << std::endl;
		std::cout << "Inicio: " << rangesURL[i].i << " | Final: " << rangesURL[i].e << std::endl;
	}
	int dif;
	for (size_t i = 0; i < rangesLoc.size(); i++)
	{
		dif = cmp_str_ranges(loc, rangesLoc[i].i, rangesLoc[i].e - rangesLoc[i].i + 1,
				url, rangesURL[i].i, rangesURL[i].e - rangesURL[i].i + 1);
		if (dif != 0)
			return (dif);
	}
	return (0);
}

int	main(void)
{
	std::string	loc("/buenas/eee/holas/");
	std::string	url("/buenas/eee/holas/buenas/aaa");
	if (!cmpLocUrl(loc, url))
		std::cout << "URL está contenida en location" << std::endl;
	else
		std::cout << "URL no está contenida en location, mal" << std::endl;
	return (1);
}
