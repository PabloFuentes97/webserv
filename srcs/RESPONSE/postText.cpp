#include "../../includes/webserv.hpp"

void	postText(std::string &route, client &client)
{
	std::string filename = client.request.url.substr(client.loc->context._args[0].length() + 1,
							client.request.url.length() - client.loc->context._args[0].length());
	if (filename.empty() || std::strchr(filename.c_str(), '/'))
		throw (BAD_REQUEST);
	if (!access((route + filename).c_str(), F_OK))
		throw (CONFLICT);
	const char *body = client.request.buf.c_str();
	std::ofstream	newfile(route + filename, std::ios::binary | std::ios::trunc);
	for (size_t i = 0; i < client.request.bufLen; i++)
		newfile << body[i];
}