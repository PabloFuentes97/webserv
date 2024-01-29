#include "../../includes/webserv.hpp"

client *findClientFd(std::vector<client> &clients, int fd)
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].fd == fd)
			return (&clients[i]);
	}
	return (NULL);
}

int	findPortbySocket(t_ports *ports, int socket)
{
	for (size_t i = 0; i < ports->n; i++)
	{
		if (ports->fd[i] == socket)
			return (ports->id[i]);
	}
	return (-1);
}

parseTree *findServerByClient(std::vector<parseTree *> servers, struct client *client)
{
	std::multimap<std::string, std::string>::iterator	it;
	//std::cout << "BUSCAR POR HOSTNAME" << std::endl;
	//Busca por hostname
	it = client->request.headers.find("hostname");
	if (it != client->request.headers.end())
	{
		std::string &serverName = it->second;
		//busca por servername
		//std::cout << "SERVER_NAME: " << serverName << std::endl;
		for (size_t i = 0; i < servers.size(); i++)
		{
			it = servers[i]->context._dirs.find("server_name");
			if (it != servers[i]->context._dirs.end())
			{
				if (it->second == serverName)
					return (servers[i]);
			}
		}
	}
	//Busca por puerto
	//std::cout << "BUSCAR POR PUERTO" << std::endl;
	for (size_t i = 0; i < servers.size(); i++)
	{
		it = servers[i]->context._dirs.find("listen");
		if (it != servers[i]->context._dirs.end())
		{
			int	port = atoi(it->second.c_str());
			if (port == client->portID)
				return (servers[i]);
		}
	}
	return (NULL);
}

pollfd *findUnusedPoll(pollfd *polls, int polls_n)
{
	for (int i = 0; i < polls_n; i++)
	{
		if (polls[i].fd == -1)
			return (&polls[i]);
	}
	return (NULL);
}


void	setEvent(pollfd *event, int _fd, short _event, short _revent)
{
	event->fd = _fd;
	event->events = _event;
	event->revents = _revent;
}

void	deleteClient(std::vector<client> &clients, client &c) //int i, int events_n
{
	close(c.fd);
	setEvent(c.events[0], -1, 0, 0);
	setEvent(c.events[1], -1, 0, 0);
	std::vector<client>::iterator	it = std::find(clients.begin(), clients.end(), c);
	if (it != clients.end())
	{
		//std::cout << "Ha encontrado un cliente para borrar: " << it->fd << std::endl;
	}
	clients.erase(it);
	std::cerr << "Elimina cliente" << std::endl;
	/*
	if (i == events_n)
	{
		for ( ; events[i].fd == -1 && i >= 0; i--, events_n);
	}*/
	//n--;
}

size_t	getTimeSeconds()
{
	time_t	seconds;

	seconds = time(NULL);
	return (seconds);
}

void	setClient(struct client &client, int fd, int id, std::vector<parseTree*> servers)
{
	client.fd = fd;
	client.portID = id;
	client.state = 0;
	client.request.cgi = 0;
	client.request.bufLen = 0;
	client.response.bytesSent = 0;
	client.server = findServerByClient(servers, &client);
	client.loc = NULL;
	client.timer = getTimeSeconds();
}

int	createClient(std::vector<client> &clients, std::vector<parseTree *> servers, int socket, t_ports *ports, pollfd *events, int &events_n)
{
	struct sockaddr_in	client_addr;
	int					client_len;

	//std::cerr << "Crea cliente nuevo" << std::endl;
	//std::cout << "SOCKET DE ESCUCHA: " << socket << std::endl;
	int accept_socket = accept(socket, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
	//std::cout << "Socket de cliente aceptado: " << accept_socket << std::endl;
	if (accept_socket == -1)
	{
		std::cout << "ACCEPT ERROR" << std::endl;
		return (0); //throw
	}
	setNonBlocking(accept_socket);
	//CLIENTE
	client c;
	setClient(c, accept_socket, findPortbySocket(ports, socket), servers);
	//std::cout << "Añadir cliente al vector" << std::endl;
	
	//std::cout << "Añadió bien el cliente al vector" << std::endl;
	short	poll_events[2] = {POLLIN, POLLOUT};
	for (int e = 0; e < 2; e++)
	{
		pollfd *event = findUnusedPoll(events, events_n);
		if (event)
		{
			//std::cout << "Hay evento a -1 para sobreescribir";
			setEvent(event, accept_socket, poll_events[e], 0);
			c.events[e] = event;
			std::cout << "NEW CLIENT | FD: " << event->fd << " | EVENTS: " << event->events << std::endl;
		}
		else
		{
			if (events_n < SOMAXCONN)
			{
				//std::cout << "No hay evento a -1 para sobreescribir, escribo en el máximo actual" << std::endl;
				setEvent(&events[events_n], accept_socket, poll_events[e], 0);
				c.events[e] = &events[events_n];
				std::cout << "NEW CLIENT | FD: " << events[events_n].fd << " | EVENTS: " << events[events_n]
					.events << std::endl;
				events_n++;
			}
		}
	}
	clients.push_back(c);
	return (1);
}

int	readClient(std::vector<client> &clients, std::vector<parseTree *> servers, pollfd &event)
{
	client *curr_client = findClientFd(clients, event.fd);
	try
	{
		//develop
    if (curr_client && curr_client->state < 2)
		{
			std::cout << "READ EVENT" << std::endl;
			if (readEvent(curr_client))
				deleteClient(clients, *curr_client);
			else 
			{
				//std::cout << "Estado de cliente tras leer: " << curr_client->state << std::endl;
				if (curr_client->state == 2)
				{
					setEvent(&event, -1, 0, 0);
					curr_client->server = findServerByClient(servers, curr_client);
					if (!curr_client->server)
						deleteClient(clients, *curr_client);
					else
						ResponseToMethod(curr_client);
				}
			}
		}
	}
	catch(const int error)
	{
		curr_client->request.status = error;
		getErrorResponse(curr_client, error);
		curr_client->state = 3;
	}
	
	
	//CATCH	CODIGO DE ERROR
	/*
		ESCRIBIR HEADER + HTML DE ERROR	curr_client.response.response = "";
		PONER ESTADO DEL CLIENTE A ESPERAR PARA RESPONDER (curr_client->state = 3)
	*/
	return (1);
}

int	checkTimerExpired(std::vector<client> &clients)
{
	//std::list<client>	deleteQueue;
	for (size_t i = 0; i < clients.size(); i++)
	{
		std::string *timeoutStr = getMultiMapValue(clients[i].server->context._dirs, "timeout");
		if (timeoutStr)
		{
			size_t	timeoutInt = atoi(timeoutStr->c_str());
			size_t	time = getTimeSeconds();
			if ((time - clients[i].timer) >= timeoutInt && clients[i].state < 2)
			{
				std::cout << "Te pasaste de tiempo" << std::endl;
				std::cout << "CLIENT: " << clients[i].fd << std::endl;
				std::cout << "Timeout: " << timeoutInt << std::endl;
				std::cout << "Client timer: " << clients[i].timer << std::endl;
				std::cout << "Time: " << time << std::endl;
				std::cout << "Tiempo que ha pasado: " << time - clients[i].timer << std::endl;
				clients[i].request.status = 408;
				getErrorResponse(&clients[i], 408);
				setEvent(clients[i].events[0], -1, 0, 0);
				clients[i].state = 3;
			}
		}
	}
		
	return (0);
}

int	pollEvents(std::vector<parseTree *> &servers, t_ports *ports)
{
	//std::cout << "EN POLLEVENTS" << std::endl;
	pollfd	events[SOMAXCONN + 1];
	int		events_n = ports->n;
	std::vector<client>	clients;
	bzero(events, sizeof(pollfd) * (SOMAXCONN + 1));
	for (size_t i = 0; i < ports->n; i++)
		setEvent(&events[i], ports->fd[i], POLLIN, 0);

	int	sign_events;

	for (;;)
	{
		sign_events = poll(events, events_n, 0);
        if (sign_events == -1) {
            perror("poll failed: ");
        }
		if (sign_events > 0)
		{
			int	events_it = events_n;
			for (int i = 0, j = 0; i < events_it && j < sign_events; i++)
			{
				checkTimerExpired(clients);
				if (events[i].revents & POLLIN) // EVENTO DE LECTURA
				{
					if (events[i].fd < (int)(ports->n + 3)) //ES SOCKET DE PUERTO, CREAR NUEVO CLIENTE
					{
						createClient(clients, servers, events[i].fd, ports, events, events_n);
						//continue ;
					}
					else // SOCKET DE CLIENTE, LEER REQUEST
						readClient(clients, servers, events[i]);
					j++;
				}
				else if (events[i].revents & POLLOUT) // SOCKET DE CLIENTE, ESCRIBIR REQUEST
				{
					client *curr_client = findClientFd(clients, events[i].fd);
					if (curr_client && curr_client->state == 3)
					{
						std::cout << "WRITE EVENT" << std::endl;
						if (writeEvent(curr_client) <= 0)
						{
							std::cout << "Termina de escribir, BORRA" << std::endl;
							deleteClient(clients, *curr_client);
						}		
					}
					j++;
				}		
			}
		}
	}
	return (1);
}	