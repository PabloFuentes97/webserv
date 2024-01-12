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

void	postMultiPartForm(std::string route, const char *body, std::string boundary, size_t size)
{
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
		if (c >= 2 && fheader[c - 1] == '\n' && fheader[c - 2] == '\n')
		{
			limit = locate_boundary(body, ("\n--" + boundary).c_str(), i, size, ("\n--" + boundary).size());
			while (i < limit)
			{
				fcont.push_back(body[i]);
				i++;
			}
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
	if (locate_boundary(body, ("\n--" + boundary + "--").c_str(), i - c, size, ("\n--" + boundary + "--").size()) != i - ("\n--" + boundary + "--").size())
		throw("no final boundary");
	create_files(files, route);
}
