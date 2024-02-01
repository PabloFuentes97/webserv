#include "../includes/webserv.hpp"

int	setPorts(t_ports &ports, std::vector<parseTree *> &servers)
{
	int					id;
	struct sockaddr_in	addr;

	ports.n = 0;
	for (size_t i = 0; i < servers.size(); i++)
	{
		std::string	*value = getMultiMapValue(servers[i]->context._dirs, "listen");
		if (!value)
			return (1);
		id = atoi(value->c_str());
		bool rep = true;
		for (size_t j = 0; j < ports.n; j++)
		{
			if (ports.id[j] == id)
			{
				rep = false;
				break ;
			}
		}
		if (rep == true)
		{
			std::cout << "Port de server " << i << " es: " << id << std::endl;
			ports.id.push_back(id);
			ports.fd.push_back(getServerSocket(&addr, id));
			std::cout << "Socket de servidor " << i << " es: " << ports.fd.back() << std::endl;
			bindAndListen(ports.fd[i], &addr);
			ports.n++;
		}
	}
	return (0);
}


int	main(int argc, char **argv) {

	std::string file;
	if (argc > 2)
	{
		std::cerr << "Format: ./webserv [configuration file]" << std::endl;
		return (1);
	}
	else if (argc == 1)
	{
		std::cout << "Using default config file" << std::endl;
		file = "configs/min_config.txt";
	}
	else
		file = argv[1];
    if (access(file.c_str(), R_OK) != 0)
	{
        std::cerr << "Inaccessible file" << std::endl;
        return (2);
	}
	
	parseTree	*root = parseFile((char *)file.c_str());
	if (!root)
	{
		std::cout << "BAD CONFIG FILE " << std::endl;
		return (3);
	}	
	parseTree	*http = NULL;
	findNode(root, &http, "http");
	if (!http)
		return (4);

	std::vector<parseTree*>	servers;
	for (size_t i = 0; i < http->childs.size(); i++)
	{
		if (http->childs[i]->context._name == "server")
			servers.push_back(http->childs[i]);
	}
	t_ports	ports;
	setPorts(ports, servers);
	pollEvents(servers, &ports);	
	return (0);
}

