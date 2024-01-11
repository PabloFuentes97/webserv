#include "webserv.hpp"

int	delete_method(int	socket, bTreeNode	*server, std::string &loc)
{
	struct stat	st;

	std::cout << "Delete method" << std::endl;
	bTreeNode	*find_loc = NULL;
	if (access(&loc.c_str()[1], F_OK) < 0) //hacer una función generica de search o find y pasar una funcion como arg
	{
		std::cout << "NOZING BORRAO\n";
		return (404);
	}	
	
	

	if (stat(loc.c_str(), &st) == 0 && !(st.st_mode & S_IFDIR))
    {
		std::cout << "borrao DIR\n";
		if (rmdir(&loc.c_str()[1]) < 0)
			return (404);
	}
	else
	{
		std::cout << "borrao FILE\n";
		if (remove(&loc.c_str()[1]) < 0)
			return (404);
	}
	char	response[] = "HTTP/1.1 200 OK\r\n\r\n<h1>File deleted.</h1>\n  </body>\n</html>\n";
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