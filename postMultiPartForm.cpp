#include "webserv.hpp"

size_t locate_boundary(const char *haystack, const char *needle, size_t i, size_t size, size_t nsize)
{
	size_t	n = i;
	size_t	e;

	while (n < size)
	{
		e = 0;
		while (haystack[n] == needle[e])
		{
			if (n == size)
				throw ("boundary not found");
			e++;
			n++;
			if (e == nsize)
				return (n - e);
		}
		n = n - e;
		n++;
	}
	throw ("boundary not found");
}

std::string	get_filename(std::vector<char> &header)
{
	std::string	filename;
	char cheader[header.size()];
	for (size_t i = 0; i < header.size(); i++)
		cheader[i] = header[i];
	size_t start = locate_boundary(cheader, "filename=\"", 0, header.size(), 10) + 10;
	while (start < header.size() && header[start] != '\"')
	{
		filename += cheader[start];
		start++;
	}
	if (header[start] != '\"')
		throw ("wrong format");
	return (filename);
}

void	create_files(std::map<std::string, std::vector<char> >files, std::string route)
{
	for (std::map<std::string, std::vector<char> >::iterator iter = files.begin(); iter != files.end(); iter++)
	{
		std::ofstream	newfile(route + iter->first, std::ios::binary | std::ios::trunc);
		for (size_t i = 0; i < iter->second.size(); i++)
			newfile << iter->second[i];
		newfile.close();
	}
}

void	postMultiPartForm(std::string &route, const char *body, std::string &boundary, size_t size)
{
	std::cout << "Entra en postMultiPartForm" << std::endl;
	std::cout << "route: \n" << route << "\nbody: \n";
	for(size_t t = 0; t < size; t++)
		std::cout << body[t];
	std::cout << "\nboundary: \n"<< boundary << "\nsize: \n" << size << std::endl;
	std::map<std::string, std::vector<char> >	files;
	std::vector<char>	fheader;
	std::vector<char>	fcont;
	std::string			filename;
	size_t		limit;
	size_t		i = 0;
	size_t		c = 0;

	while (i < size)
	{
		fheader.push_back(body[i]);
		i++;
		c++;
		if (c >= 4 && fheader[c - 1] == '\n' && fheader[c - 2] == '\r' && fheader[c - 3] == '\n' && fheader[c - 4] == '\r')
		{
			limit = locate_boundary(body, ("\n--" + boundary).c_str(), i, size, ("\n--" + boundary).size() - 1);
			std::cout << "\nSTART DE LIMIT\n";
			for (size_t y = limit; y < limit + ("\r\n--" + boundary).size() - 1; y++)
				std::cout << body[y];
			std::cout << "\nFIN DE LIMIT\n";
			while (i < limit)
			{
				std::cout << body[i];
				fcont.push_back(body[i]);
				i++;
			}
			std::cout << "\nFIN DEL CONTENT\n";
			filename = get_filename(fheader);
			if (!access((route + filename).c_str(), F_OK))
				throw("already exists");
			for (std::map<std::string, std::vector<char> >::iterator iter = files.begin(); iter != files.end(); iter++)
				if (filename == iter->first)
					throw("can't upload more than one file with the same name");
			files.insert(std::pair<std::string, std::vector<char> >(filename, fcont));
			fcont.clear();
			fheader.clear();
			c = 0;
		}
	}
	/* if (locate_boundary(body, ("\n--" + boundary + "--").c_str(), i - c, size, ("\r\n--" + boundary + "--").size()) != i - ("\r\n--" + boundary + "--").size())
		throw("no final boundary"); */
	create_files(files, route);
}

int	callMultiPart(struct client *client, std::string &path)
{
	std::cout << "Entra en callMultiPart" << std::endl;
	std::pair <std::multimap<std::string, std::string>::iterator, std::multimap<std::string, std::string>::iterator> itm;
	itm = client->request.headers.equal_range("Content-Type");
	char	*boundary = NULL;
	for (std::multimap<std::string, std::string>::iterator itb = itm.first; itb != itm.second; itb++)
	{
		std::cout << "Key: " << itb->first << " Value: " << itb->second << std::endl;
		if (!itb->second.compare(0, 9, "boundary="))
		{
			boundary = (char *)(itb->second.c_str() + 9);
			std::cout << "Encuentro el boundary: " << boundary;
			break ;
		}
	}
	if (!boundary)
		return (0);
	std::string	boundaryStr(boundary);
	try
	{
		postMultiPartForm(path, client->request.buf.c_str(), boundaryStr, client->request.bufLen);
	}
	catch(char const *s)
	{
		std::cerr << s << '\n';
	}	
	return (1);	
}
