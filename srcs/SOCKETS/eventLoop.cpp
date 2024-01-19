#include "../../includes/webserv.hpp"

bool	getValue(std::vector<std::pair<std::string, std::vector<std::string> > > keyValues, std::string key, std::vector<std::string>	*values_out)
{
	for (size_t i = 0; i < keyValues.size(); i++)
	{
		if (keyValues[i].first == key)
		{
			*values_out = keyValues[i].second;
			return (true);
		}		
	}
	return (false);
}

client *findClientFd(std::vector<client> &clients, int fd)
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].fd == fd)
			return (&clients[i]);
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

int	pollEvents(std::vector<bTreeNode *> servers, std::vector<int>	&sockets)
{
	pollfd	events[SOMAXCONN + 1];
	int		events_n = sockets.size();
	std::vector<client>	clients;
	bzero(events, sizeof(pollfd) * (SOMAXCONN + 1));
	for (size_t i = 0; i < sockets.size(); i++)
	{
		events[i].fd = sockets[i];
		events[i].events = POLLIN;
		//events[i].revents = -1;
	}
	
	int	sign_events;
	int	accept_socket;
	//struct sockaddr_in	addr;
	struct sockaddr_in	client_addr;
	int					client_len;

	for (;;) {
		//std::cout << "----------LLAMAR A POLL PARA BUSCAR NUEVOS EVENTOS---------" << std::endl;
		sign_events = poll(events, events_n, 0);
        if (sign_events == -1) {
            perror("poll failed: ");
            exit(1);
        }
		if (sign_events > 0)
		{
			//std::cout << "Número de eventos a monitorear: " << events_n << std::endl;
			//std::cout << "Número de eventos señalizados: " << sign_events  << std::endl;
			int	events_it = events_n;
			for (int i = 0, j = 0; i < events_it && j < sign_events; i++)
			{
				//std::cout << "========EVENTO DETECTADO======" << std::endl;
				//std::cout << "EVENTO " << i << ": FD: " << events[i].fd << " | EVENTS: "
				//	<< events[i].events << " | REVENTS: " << events[i].revents << std::endl;
				if (events[i].revents & POLLIN) // socket de escucha, crear nuevo cliente
				{
					if (events[i].fd < (int)(sockets.size() + 3))
					{
						std::cout << "SOCKET DE ESCUCHA: " << events[i].fd << std::endl;
						accept_socket = accept(events[i].fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
						std::cout << "Socket de cliente aceptado: " << accept_socket << std::endl;
						if (accept_socket == -1) {
							std::cout << "ACCEPT ERROR" << std::endl;
							exit(0);
						}
						setNonBlocking(accept_socket);
						//CLIENTE
						client c;
						c.fd = accept_socket;
						c.serverID = events[i].fd - 3;
						c.state = 0;
						c.request.cgi = 0;
						c.request.bufLen = 0;
						std::cout << "Añadir cliente al vector" << std::endl;
						clients.push_back(c);
						std::cout << "Añadió bien el cliente al vector" << std::endl;
						short	poll_events[2] = {POLLIN, POLLOUT};
						for (int e = 0; e < 2; e++)
						{
							pollfd *event = findUnusedPoll(events, events_n);
							if (event)
							{
								std::cout << "Hay evento a -1 para sobreescribir";
								event->fd = accept_socket;
								event->events = poll_events[e];
								event->revents = 0;
								std::cout << "NEW EVENT | FD: " << event->fd << " | EVENTS: " << event->events << std::endl;
							}
							else
							{
								if (events_n < SOMAXCONN)
								{
									std::cout << "No hay evento a -1 para sobreescribir, escribo en el máximo actual" << std::endl;
									events[events_n].fd = accept_socket;
									events[events_n].events = poll_events[e];
									events[events_n].revents = 0;
									std::cout << "NEW EVENT | FD: " << events[events_n].fd << " | EVENTS: " << events[events_n]
										.events << std::endl;
									events_n++;
								}
							}
							//if (poll(events, events_n, 0) < 0)
							//	perror("kevent error");	
						}
						events[i].revents = 0;
						j++;
						continue ;
					}
					else //socket de cliente, leer request
					{
						client *curr_client = findClientFd(clients, events[i].fd);
						if (curr_client && curr_client->state < 2)
						{
							std::cout << "EVENTO DE LECTURA" << std::endl;
							if (readEvent(curr_client))
							{
								// FUNCION DELETE CLIENT
								events_n--;
								events[i].fd = -1;
								events[i].events = 0;
								std::vector<client>::iterator	it = std::find(clients.begin(), clients.end(), *curr_client);
								clients.erase(it);
							}
							std::cout << "Estado de cliente tras leer: " << curr_client->state << std::endl;
							if (curr_client->state == 2)
							{
								events[i].fd = -1;
								events[i].events = 0;
							}
						}
						events[i].revents = 0;
						j++;	
					}
				}
				if (events[i].revents & POLLOUT) // cliente puede escribir
				{
					client *curr_client = findClientFd(clients, events[i].fd);
					if (curr_client && curr_client->state == 2)
					{
						std::cout << "EVENTO DE ESCRITURA" << std::endl;
						writeEvent(servers[curr_client->serverID], curr_client);
						close(events[i].fd); //cerrar socket de conexion - se terminó
						// FUNCION DELETE CLIENT
						events_n--;
						events[i].fd = -1;
						events[i].events = 0;
						events[i].revents = 0;
						std::vector<client>::iterator	it = std::find(clients.begin(), clients.end(), *curr_client);
						clients.erase(it);
					}
					j++;
				}		
			}
		}
		//sleep(5);
	}
}	