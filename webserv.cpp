#include "webserv.hpp"

//
bool	getValue(std::vector<std::pair<std::string, std::vector<std::string> > > keyValues, std::string key, std::vector<std::string>	*values_out)
{
	for (int i = 0; i < keyValues.size(); i++)
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
	for (int i = 0; i < clients.size(); i++)
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
	for (int i = 0; i < sockets.size(); i++)
	{
		events[i].fd = sockets[i];
		events[i].events = POLLIN;
		//events[i].revents = -1;
	}
	
	int	sign_events;
	int	accept_socket;
	struct sockaddr_in	addr;
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
					if (events[i].fd < sockets.size() + 3)
					{
						std::cout << "SOCKET DE ESCUCHA: " << events[i].fd << std::endl;
						accept_socket = accept(events[i].fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
						std::cout << "Socket de cliente aceptado: " << accept_socket << std::endl;
						if (accept_socket == -1) {
							std::cout << "ACCEPT ERROR" << std::endl;
							exit(0);
						}
						setNonBlocking(accept_socket);
						client c = {accept_socket, events[i].fd - 3, 0};
						clients.push_back(c);
						std::cout << "Añadió cliente al vector " << std::endl;
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
							readEvent(curr_client);
							events[i].revents = 0;
							std::cout << "Estado de cliente tras leer: " << curr_client->state << std::endl;
							
							if (curr_client->state == 2)
							{
								events[i].fd = -1;
								events[i].events = 0;
								events[i].revents = 0;
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
						//events[i] = events[events_n];
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

int	kqueue_events(std::vector<bTreeNode *>servers, std::vector<int> &sockets)
{
	struct kevent event[SOMAXCONN + 1];

	int kq = kqueue();
	if (kq < 0) {
    	perror("kqueue()");
    	exit (1);
  	}
	
	//Declarar la clientQueue;
	clientQueue Queue;
	std::vector<client>	clients;
	//Seter el evento para la kqueue
	struct kevent server_event;
	for (int i = 0; i < servers.size(); i++) {
		EV_SET(&server_event, sockets[i], EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, NULL);
		//Registrar el evento
		if (kevent(kq, &server_event, 1, NULL, 0, NULL) == -1) {
    	    perror("kevent");
    	    exit(1);
   		}
	}
	//Montar el cliente en el vector
	//Chequear para evento una vez cada ciclo
	int new_events;
	int	new_sock;
	struct sockaddr_in	addr;
	struct sockaddr_in	client_addr;
	int					client_len;
	for (;;) {
		std::cout << "----------LLAMAR A KEVENT PARA BUSCAR NUEVOS EVENTOS---------" << std::endl;
		new_events = kevent(kq, NULL, 0, event, SOMAXCONN + 1, NULL);
		std::cout << "Número de eventos señalizados: " << new_events << std::endl;
        if (new_events == -1) {
            perror("kevent");
            exit(1);
        }
		for (int i = 0; i < new_events; i++) {
			struct kevent client_event[2];
			std::cout << "========EVENTO DETECTADO======" << std::endl;
			/*if (event[i].udata != NULL)
			{
				std::cout << "EVENTO ES SOCKET DE CLIENTE" << std::endl;
				client *clientPtr = (client *)event[i].udata;
				std::cout << "CLIENT FD: " << clientPtr->fd << " | SERVER ID: " << clientPtr->serverID
						<< " | STATE: " << clientPtr->state << std::endl;
			}*/
			//Socket de escucha
			bool isServerSocket = false;
			if (event[i].ident <= sockets.size() + 3)
			{
				std::cout << "EVENTO ES SOCKET DE ESCUCHA: " << event[i].ident << std::endl;
				isServerSocket = true;
			}
			if (isServerSocket == true && event[i].filter == EVFILT_READ) {

				std::cout << "---ACCEPT EVENT---" << std::endl;
                //Montar cliente
                new_sock = accept(event[i].ident, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
				if (new_sock == -1) {
					std::cout << "ACCEPT ERROR" << std::endl;
					exit(0);
				}
				setNonBlocking(new_sock);
				client c = {new_sock, event[i].ident - 3, 0};
				clients.push_back(c);
				//Queue.addClient(new_sock, event[i].ident);
				//std::cout << "ADDED CLIENT. FD: " << event[i].ident << "SERVER ID: " << Queue.getServerId(new_sock) << std::endl;
				std::cout << "ADDED CLIENT. FD: " << clients.back().fd << " | SERVER ID: " << clients.back().serverID << std::endl;
                // Nuevo evento al kqueue
				EV_SET(&client_event[0], new_sock, EVFILT_READ, EV_ADD | EV_CLEAR | EV_EOF, 0, 0, &clients.back());
				EV_SET(&client_event[1], new_sock, EVFILT_WRITE, EV_ADD | EV_DISABLE | EV_CLEAR, 0, 0, &clients.back());
				if (kevent(kq, client_event, 2, NULL, 0, NULL) < 0)
                    perror("kevent error");
				continue ;
			}
			client *clientPtr = (client *)event[i].udata;
			if (isServerSocket == false && event[i].filter == EVFILT_WRITE && clientPtr->state == 2) // && clientPtr->state == 2
			{
				std::cout << "EVENTO DE ESCRITURA" << std::endl;
				//writeEvent(servers[event[i].ident - 3], &event[i]);
				//writeEvent(servers[Queue.getServerId(event[i].ident)], Queue, event[i].ident, client_event, kq);
				close(event[i].ident); //cerrar socket de conexion - se terminó
				//std::vector<client>::iterator	it = std::find(clients.begin(), clients.end(), *clientPtr);
				//clients.erase(it);
			}
			else if (isServerSocket == false && event[i].filter == EVFILT_READ && clientPtr->state < 2) // && clientPtr->state < 2
			{
				std::cout << "EVENTO DE LECTURA" << std::endl;
				//readEvent(&event[i]);
				//readEvent(Queue, &event[i], client_event, kq); //event es el evento que ha ocurrido; client_event es el tipo de evento a monitorear
				std::cout << "Estado de cliente tras leer: " << clientPtr->state << std::endl;
				if (clientPtr->state == 2)
				{
					EV_SET(&client_event[1], event[i].ident, EVFILT_WRITE, EV_ENABLE | EV_CLEAR, 0, 0, NULL);
					if (kevent(kq, &client_event[1], 1, NULL, 0, NULL) == -1)
						std::cerr << "kevent error" << std::endl;
				}
			}
			
		}
	}
	return (0);
}

int	main(int argc, char **argv) {

	if (argc != 2) {
        std::cerr << "Format: ./webserv [configuration file]" << std::endl;
        return (1); }
    if (access(argv[1], R_OK) != 0) {
        std::cerr << "Inaccessible file" << std::endl;
        return 1; }
	
	

	std::vector<int>	sockVec;
	
	struct sockaddr_in	addr;
	struct sockaddr_in	client_addr;
	int					client_len;

	//guardar arbol con config
	bTreeNode	*root = parseFile(argv[1]);
	if (!root)
		return (2);
	bTreeNode	*http = NULL;
	findNode(root, &http, "http");
	if (!http)
		return (2);
	std::vector<bTreeNode*>	servers;
	for (int i = 0; i < http->childs.size(); i++)
	{
		if (http->childs[i]->contextName == "server")
			servers.push_back(http->childs[i]);
	} 
	//Socket del servidor
	for (int i = 0; i < servers.size(); i++)
	{
		int	port;
		std::vector<std::string>	values;
		if (!getValue(servers[i]->directives, "listen", &values))
			return (3);
		port = atoi(values[0].c_str());
		std::cout << "Port de server " << i << " es: " << port << std::endl;
		sockVec.push_back(getServerSocket(&addr, port));
		std::cout << "Socket de servidor " << i << " es: " << sockVec.back() << std::endl;
		bindAndListen(sockVec[i], &addr);
	}
	pollEvents(servers, sockVec);	
	return 0;
}

