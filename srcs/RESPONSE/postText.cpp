#include "../../includes/webserv.hpp"

void	postText(std::string &route, const char *body, size_t size, std::string	filename)
{
	if (!access((route + filename).c_str(), F_OK))
		throw (CONFLICT);
	std::ofstream	newfile(route + filename, std::ios::binary | std::ios::trunc);
	for (size_t i = 0; i < size; i++)
		newfile << body[i];
}