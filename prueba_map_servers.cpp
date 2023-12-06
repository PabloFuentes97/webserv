#include "webserv.hpp"

int	pruebaMapTreeServers(bTreeNode *root)
{
	servers	prueba;

	prueba.servers_n = 2;
	//servidor
	int	fd_num = 6;
	int	servers_num = 0;
	int	clients_num = 0;
	for (int i = 0; i < prueba.servers_n; i++, servers_num++, fd_num += 3)
	{
		for (int j = fd_num; j < fd_num + 3; j++, clients_num++)
			prueba.clients.addClient(j, i);
	}
	prueba.serversPtr = root->childs;
	srand(time(NULL));
	int	random;
	for (int i = 0; i < prueba.clients.clientArray.size(); i++)
	{
		random = rand() % 3;
		switch(random)
		{
			case 0: { prueba.clients.clientArray[i].request.url = "/juan"; break; };
			case 1: { prueba.clients.clientArray[i].request.url = "/manuel"; break; };
			case 2: { prueba.clients.clientArray[i].request.url = "/jose"; break; };
		}
	}
	for (int i = 0; i < prueba.clients.clientArray.size(); i++)
	{
		std::cout << "Cliente: " << std::endl; 
		std::cout << "Servidor: " << prueba.clients.clientArray[i].serverID << std::endl;
		std::cout << "Socket de conexión: " << prueba.clients.clientArray[i].fd << std::endl;
		std::cout << "URL: " << prueba.clients.clientArray[i].request.url << std::endl;
		findLocation(prueba.serversPtr[prueba.clients.clientArray[i].serverID], prueba.clients.clientArray[i].request.url);		
	}
	return (0);
}