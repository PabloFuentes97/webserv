#include "webserv.hpp"

int	delete_method(int	socket, bTreeNode	*server, std::string &loc)
{
	std::cout << "Delete method" << std::endl;
	bTreeNode	*find_loc = NULL;
	if (!findLocation(server, loc)) //hacer una función generica de search o find y pasar una funcion como arg
		return (404);
	if (remove(&loc.c_str()[1]) < 0)
		return (404);
	char	response[] = "HTTP/1.1 200 OK\nDate: Wed, 21 Oct 2015 07:28:00 GMT\n<html>\n  <body>\n    <h1>File deleted.</h1>\n  </body>\n</html>\n";
	write(socket, response, 114);
	return (200);
}

/*int	main(int argc, char **argv)
{
	if (argc != 2)
		return (1);
	bTreeNode	*root = parseFile(argv[1]);
	if (!root)
		return (2);
	std::string	loc = "/del_prueba";
	bTreeNode	*http = NULL;
	findNode(root, &http, "http");
	if (!http)
		return (3);
	std::vector<bTreeNode*>	servers = http->childs;
	delete_method(1, servers[0], loc);
	return (0);
}*/