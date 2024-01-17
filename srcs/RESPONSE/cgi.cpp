#include "../../includes/webserv.hpp"

std::string	CGIForward(std::string &path)
{
	std::cout << "Path del ejecutable: " << path << std::endl;
	int	pipes[2];
	pipe(pipes);
	char	**args = (char **)malloc(sizeof(char *) * 2);
	args[0] = (char *)path.c_str();
	args[1] = NULL;
	int	pid = fork();
	if (pid == 0)
	{
		std::cout << "Estoy en proceso hijo" << std::endl;
		dup2(pipes[1], STDOUT_FILENO);
		close(pipes[0]);
		if (execve("/usr/bin/python", args, NULL) < 0)
			exit(1);
	}
	free(args);
	args = NULL;
	int	status;
	waitpid(pid, &status, 0);
	std::cout << "Estoy en proceso padre" << std::endl;
	close(pipes[1]);
	char	*readCGI = readFileSeLst(pipes[0]);
	std::cout << "Fichero leído: " << readCGI << std::endl;
	return (readCGI);
}

/*int	main(int argc, char **argv)
{
	if (argc != 2)
		return (1);
	bTreeNode	*root = parseFile(argv[1]);
	bTreeNode	*server;
	findNode(root, &server, "server");
	std::string url("/");
	bTreeNode	*loc = findLocation(server, url);
	std::multimap<std::string, std::string>::iterator itm;
	itm = loc->directivesMap.find("cgi_pass");
	if (itm != loc->directivesMap.end())
	{
		std::cout << "Encontró la key: " << itm->first << " | Valor: " << itm->second << std::endl;
	}
	char	buf[1000];
	std::string absPath = getcwd(buf, 1000);
	std::string	path = absPath + itm->second;
	std::string	path = "/Users/pfuentes/ejercicios/CPP/webserv_git/cgi/cgi";
	std::string	response = CGIForward(path);
	std::cout << "Response: " << response << std::endl;
	return (0);
}*/