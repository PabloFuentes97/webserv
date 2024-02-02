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
	typedef std::multimap<std::string, std::string>::iterator	it;

	//Busca por hostname
	it ith = client->request.headers.find("Hostname");
	if (ith != client->request.headers.end())
	{
		for (size_t i = 0; i < servers.size(); i++)
		{
			it its = servers[i]->context._dirs.find("server_name");
			it itp = servers[i]->context._dirs.find("listen");
			if (its != servers[i]->context._dirs.end()
				&& itp != servers[i]->context._dirs.end())
			{
				if (!strncmp(ith->second.c_str(), its->second.c_str(), its->second.length())
						&& atoi(itp->second.c_str()) == client->portID)
					return (servers[i]);
			}
		}
	}
	//Busca por puerto
	for (size_t i = 0; i < servers.size(); i++)
	{
		ith = servers[i]->context._dirs.find("listen");
		if (ith != servers[i]->context._dirs.end())
		{
			int	port = atoi(ith->second.c_str());
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
	if (close(c.fd) == -1)
		std::cerr << "CLOSE FAIL\n";
	setEvent(c.events[0], -1, 0, 0);
	setEvent(c.events[1], -1, 0, 0);
	std::vector<client>::iterator	it = std::find(clients.begin(), clients.end(), c);
	clients.erase(it);
	//std::cerr << "Elimina cliente" << std::endl;
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

	int accept_socket = accept(socket, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
	if (accept_socket == -1)
	{
		std::cerr << "ACCEPT ERROR" << std::endl;
		return (0);
	}
	setNonBlocking(accept_socket);

	client c;
	setClient(c, accept_socket, findPortbySocket(ports, socket), servers);
	clients.push_back(c);
	short	poll_events[2] = {POLLIN, POLLOUT};
	for (int e = 0; e < 2; e++)
	{
		pollfd *event = findUnusedPoll(events, events_n);
		if (event)
		{
			setEvent(event, accept_socket, poll_events[e], 0);
			c.events[e] = event;
			std::cout << "NEW CLIENT | FD: " << event->fd << " | EVENTS: " << event->events << std::endl;
		}
		else
		{
			if (events_n < (int)(SOMAXCONN * 2 * ports->n))
			{
				setEvent(&events[events_n], accept_socket, poll_events[e], 0);
				c.events[e] = &events[events_n];
				std::cout << "NEW CLIENT | FD: " << events[events_n].fd << " | EVENTS: " << events[events_n]
					.events << std::endl;
				events_n++;
			}
		}
	}
	return (1);
}

int	readClient(std::vector<client> &clients, std::vector<parseTree *> servers, pollfd &event)
{
	client *curr_client = findClientFd(clients, event.fd);
	try
	{
		if (curr_client && curr_client->state < 2)
		{
			if (readEvent(curr_client))
				deleteClient(clients, *curr_client);
			else 
			{
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
	catch(enum statusCodes error)
	{
		curr_client->request.status = error;
		getErrorResponse(curr_client, error);
		curr_client->state = 3;
	}
	return (1);
}

int	checkTimerExpired(std::vector<client> &clients)
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		std::string *timeoutStr = getMultiMapValue(clients[i].server->context._dirs, "timeout");
		if (timeoutStr)
		{
			size_t	timeoutInt = atoi(timeoutStr->c_str());
			size_t	time = getTimeSeconds();
			if ((time - clients[i].timer) >= timeoutInt && clients[i].state < 2)
			{
				clients[i].request.status = REQUEST_TIMEOUT;
				getErrorResponse(&clients[i], REQUEST_TIMEOUT);
				setEvent(clients[i].events[0], -1, 0, 0);
				clients[i].state = 3;
			}
		}
	}
		
	return (0);
}

int	pollEvents(std::vector<parseTree *> &servers, t_ports *ports)
{
	pollfd	events[SOMAXCONN * 2 * ports->n];
	int		events_n = ports->n;
	std::vector<client>	clients;
	bzero(events, sizeof(pollfd) * (SOMAXCONN * 2 * ports->n));
	for (size_t i = 0; i < ports->n; i++)
		setEvent(&events[i], ports->fd[i], POLLIN, 0);
	int	sign_events;
	for (;;)
	{
		sign_events = poll(events, events_n, 0);
        if (sign_events == -1)
            perror("POLL FAILED:");
		if (sign_events > 0)
		{
			int	events_it = events_n;
			for (int i = 0, j = 0; i < events_it && j < sign_events; i++)
			{
				checkTimerExpired(clients);
				if (events[i].revents & POLLIN) // READ EVENT
				{
					if (events[i].fd < (int)(ports->n + 3)) // PORT SOCKET, CREATE NEW CLIENT
						createClient(clients, servers, events[i].fd, ports, events, events_n);
					else // CLIENT SOCKET, READ REQUEST
						readClient(clients, servers, events[i]);
					j++;
				}
				else if (events[i].revents & POLLOUT) // CLIENT SOCKET, WRITE RESPONSE
				{
					client *curr_client = findClientFd(clients, events[i].fd);
					if (curr_client && curr_client->state == 3)
					{
						if (writeEvent(curr_client) <= 0)
							deleteClient(clients, *curr_client);		
					}
					j++;
				}		
			}
		}
	}
	return (1);
}	