#include "webserv.hpp"

size_t	countCharinStr(const char *str, char c)
{
	int	count = 0;
	
	for (int i = 0; i < strlen(str); i++)
		if	(str[i] == c)
			count++;
	return (count);
}

int	cmpDirectories(std::string &s1, std::string &s2)
{
	int	slash1 = countCharinStr(s1.c_str(), '/');
	int	slash2 = countCharinStr(s2.c_str(), '/');
	std::cout << "Número de slashes en dir1: " << slash1 << std::endl;
	std::cout << "Número de slashes en dir2: " << slash2 << std::endl;
	if (slash1 > slash2)
	{
		std::cout << "Slashes no coinciden: slash1 es mayor que slash2" << std::endl;
		return (slash1 - slash2);
	}
	if (slash1 < slash2)
	{
		std::cout << "Slashes no coinciden: slash1 es menor que slash2" << std::endl;
		return (slash1 - slash2);
	}
	if (s2 == "/") //caso especial hacerlo bien solo para probar que este es el problema
		return (1);
	std::stringstream			streamDir1(s1);
	std::stringstream			streamDir2(s2);
	std::string					tokenDir1;
	std::string					tokenDir2;
	while (getline(streamDir1, tokenDir1, '/') && getline(streamDir2, tokenDir2, '/'))
	{
		std::cout << "Comparar: " << tokenDir1 << " y " << tokenDir2 << std::endl;
		if (tokenDir1 > tokenDir2)
		{
			std::cout << "No coinciden, token1 es mayor que token2" << std::endl;
			return (1);
		}
		else if (tokenDir1 < tokenDir2)
		{
			std::cout << "No coinciden, token1 es menor que token2" << std::endl;
			return (-1);
		}
	}
	std::cout << "Coinciden" << std::endl;
	return (0);
}

/*void    binaryInsertionDeque(std::deque<int> &deq, itdeque const &begin, itdeque const &end, int num)
{
    size_t  len = distance(begin, end);
    if (len == 0)
    {
		deq.insert(begin, num);
        return ;
    }
    size_t	find = len / 2;
    itdeque	itf = begin + find;
    if (num < *itf)
        binaryInsertionDeque(deq, begin, itf, num);
    else
        binaryInsertionDeque(deq, itf + 1, end, num);
}*/

int	binarySearch(std::vector<std::string> &src, std::string	&cmp, int (*f)(std::string &s1, std::string &s2))
{
	int	start = 0;
	int	end = src.size() - 1;
	int	low = start;
	int	high = end;
	int	mid;

	while (low <= high)
	{
		mid = (high + low) / 2;
		std::cout << "Low: " << src[low] << std::endl;
		std::cout << "High: " << src[high] << std::endl;
		std::cout << "Valor a comparar: " << src[mid] << " en pos: " << mid << std::endl;
		int	res = f(cmp, src[mid]);
		std::cout << "Res es: " << res << std::endl;
		if (res == 0)
		{
			std::cout << "Coinciden" << std::endl;
			return (mid);
		}
   		else if (res > 0)
		{
			std::cout << "Cmp es mayor que mid, me muevo a lado derecho" << std::endl;
			low = mid + 1;
		}	
    	else
		{
			std::cout << "Cmp es menor que mid, me muevo a lado izquierdo" << std::endl;
			high = mid - 1;
		}
	}
	if (low <= start) //iterador ha llegado a principio del array
	{
		std::cout << "No encuentra elemento, pero debería estar al principio" << std::endl;
		return (-1);
	}
	else if (high >= end) //iterador ha llegado a final del array
	{
		std::cout << "No encuentra elemento, pero debería estar al final" << std::endl;
		return (-2);
	}
	return (low);
}

int	main(void)
{
	std::string	dir = "/Users/pfuentes/ejercicios/CPP/webserver_git";
	std::string	file = "webserv";
	std::string	location1 = "/kapouet/bueno";
	std::string	location2 = "/kapouet";
	std::string	location3 = "/kapouet/pepe/pedro";
	std::string	location4 = "/kapouet/pepe";
	std::string	location5 = "/pepe/manuel/manolo/jose";
	std::string	location6 = "/";
	std::string	location7 = "/bueno";
	std::string	location8 = "/pepe/manuel";
	//std::string	URI = "/kapouet/pouic/toto/pouet";
	//std::string	URI = "/";
	//std::string	URI = "/pepe/manuel/";
	std::vector<std::string>	locations;
	locations.push_back(location1);
	locations.push_back(location2);
	locations.push_back(location3);
	locations.push_back(location4);
	locations.push_back(location5);
	locations.push_back(location6);
	locations.push_back(location7);
	locations.push_back(location8);
	std::vector<std::string>	vec;
	vec.push_back(location1);
	for (int i = 1; i < locations.size(); i++)
	{
		std::cout << "----------------------Debe guardar: " << locations[i] << "---------------------" << std::endl;
		int	res = binarySearch(vec, locations[i], &cmpDirectories);
		if (res == -1)
		{
			std::cout << "Lo añade al principio" << std::endl;
			vec.insert(vec.begin(), locations[i]);
		}
		else if (res == -2)
		{
			std::cout << "Lo añade al final" << std::endl;
			vec.push_back(locations[i]);
		}
		else
		{
			std::cout << "Guardar elemento en pos: " << res << std::endl;
			vec.insert(vec.begin() + res, locations[i]);
		}
		std::cout << "Vector que debería estar ordenado" << std::endl;
		for (int i = 0; i < vec.size(); i++)
			std::cout << vec[i] << std::endl;
	}
	//cmpDirectories(URI, location2);
	//cmpDirectories(URI, location1);
	std::cout << "----------------AL FINAL----------------" << std::endl;
	std::cout << "Vector que debería estar ordenado" << std::endl;
	for (int i = 0; i < vec.size(); i++)
		std::cout << vec[i] << std::endl;
	return (0);

}
