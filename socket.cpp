#include "webserv.hpp"

int	handle_connection(int fd)
{
	//leer solicitud del cliente
	std::cout << "Gestionar solicitud del cliente" << std::endl;
	
	//seLst	lst = {newseNode(), lst.head, 0};

	/*lst.head = newseNode();
	lst.tail = lst.tail;
	lst.size = 0;*/
	//int	lines = 0;
	char	*file = readFileSeLst(fd);
	free(file);
	/*
	char	keys[lines + 1];
	bzero(keys, lines + 1);
	char	values[lines + 1];
	bzero(values, lines + 1);*/


	/*char buf[1000];
	while (recv(fd, buf, sizeof(buf), 0) > 0)
	{
		printf("%s", buf);
		if (buf[999] == '\n')
			break ;
		bzero(buf, sizeof(buf));
	}
	READ
	char	buffer[1000];
	size_t	bytesRead = read(fd, buffer, 1000);
	buffer[999] = '\0';
	if (bytesRead < 0)
		return (5);
	*/
	//escribir respuesta al cliente
	char response[] = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nKeep-Alive: timeout=5, max=100\r\n\r\nque pasa";
	//write(fd, response, strlen(response));
	send(fd, response, strlen(response), 0); //usar send en vez de write
	/* WRITE
	size_t	bytesSend;
	write(connSocketFd, response.c_str(), 9);
	if (bytesSend != response.size())
		std::cout << "Respuesta enviada correctamente al cliente" << std::endl;
	else
		std::cout << "Error al enviar respuesta al cliente" << std::endl;
	*/
	return (1);
}

int	connectionSelect(int listenSocket)
{
	//conseguir direccion del cliente 
	struct sockaddr_in	clientAddress;
	socklen_t			clientAddressLen;
	char				clientAddressStr[1000];
	
	//preparar select, sockets actuales y sockets ya listos para leer/escribir
	fd_set				currSockets;
	fd_set				readySockets;
	FD_ZERO(&currSockets); //rellena de ceros 
	FD_SET(listenSocket, &currSockets); //añade el socket a grupo de fds
	int	connSocketFd;

	while (1)
	{
		readySockets = currSockets; //copia temporal porque select modifica/destruye el set, deja solo los fds listos, borra el resto
		if (select(FD_SETSIZE, &readySockets, NULL, NULL, NULL) < 0)
		{
			perror("Select failed: ");
			return (0);
		}
		//iterar sobre los fds que ya están listos
		for (int i = 0; i < FD_SETSIZE; i++)
		{
			if (FD_ISSET(i, &readySockets)) // comprobar que sí está listo
			{
				if (i == listenSocket) //el fd listo es el socket del servidor, hay alguna conexion entrante lista
				{
					//aceptamos la conexión porque sabemos que está lista para leerla
					connSocketFd = accept(listenSocket, (sockaddr*)&clientAddress, &clientAddressLen); //(sockaddr*)&sock, (socklen_t *)(sizeof(sock))
					FD_SET(connSocketFd, &currSockets); //añadimos socket de conexion con el cliente a lista de sockets actuales
				}
				else
				{
					handle_connection(i); //gestionar conexion, leer y escribir
					FD_CLR(i, &currSockets);  //borra el fd del set de fds
					//close(connSocketFd);
				}
			}
		}
	}
	return (1);
}

/*int	connectionEpoll(int listenSocket)
{
	int	epollFd;

	epollFd = epoll_wait
	return (1);
}*/

void	realloc_free(void **ptr, size_t len)
{
	void	*realloc_ptr = realloc(*ptr, len);
	free(*ptr);
	*ptr = realloc_ptr;
}
int	connectionPoll(int listenSocket)
{
	//conseguir direccion del cliente 
	struct sockaddr_in	clientAddress;
	socklen_t			clientAddressLen;
	char				clientAddressStr[1000];
	
	/*pollfd	*pollfds;
	int		poll_n;
	
	poll_n = 1;
	pollfds = (pollfd *)malloc(sizeof(pollfd));
	pollfds[0].fd = clientSocket;
	pollfds[0].events = POLL_IN;*/
	pollfd	pollfds[1000];
	bzero(pollfds, 1000);
	int		poll_n = 1;
	pollfds[0].fd = listenSocket;
	pollfds[0].events = POLL_IN;
	int		clientSocket;

	while (1)
	{
		if (poll(pollfds, 10, -1) < 0)
		{
			perror("Poll failed: ");
			return (0);
		}
		//iterar sobre los fds que ya están listos
		for (int i = 0; i < poll_n; i++)
		{
			std::cout << "Posición en array de poll: " << i << std::endl;
			if (pollfds[i].fd == listenSocket) //el fd listo es el socket del servidor, hay alguna conexion entrante lista
			{
				std::cout << "Listen socket" << std::endl;
				if (pollfds[i].revents == POLL_IN)
				{
					std::cout << "Socket de escucha devuelve POLL_IN" << std::endl;
					std::cout << "Revent: " << pollfds[i].revents << std::endl; 
					//aceptamos la conexión porque sabemos que está lista para leerla
					clientSocket = accept(listenSocket, (sockaddr*)&clientAddress, &clientAddressLen); //(sockaddr*)&sock, (socklen_t *)(sizeof(sock))
					if (clientSocket < 0)
						perror("Accept failed");
					//realloc_free((void **)&pollfds, poll_n + 1);
					pollfds[poll_n].fd = clientSocket;
					pollfds[poll_n].events = POLL_IN;
					pollfds[i].revents = 0;
					poll_n++;
				}
			}
			else if (pollfds[i].fd > 0)
			{
				std::cout << "Socket de conexión, leer y escribir" << std::endl;
				handle_connection(pollfds[i].fd); //gestionar conexion, leer y escribir
				close(pollfds[i].fd);
				pollfds[i].fd = -1;
				pollfds[i].events = 0;
			}
		}
	}
	return (1);
}

int	connectionKQueue(int listenSocket)
{
	int	kq = kqueue();
	if (kq == - 1)
		return (-1);

	struct kevent	evSet; //eventos a monitorear
	struct kevent	evList[100]; //eventos 'triggereados'
 
	EV_SET(&evSet, listenSocket, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, NULL);
	int	ret = kevent(kq, &evSet, 1, NULL, 0, NULL);
	int	currFd; //fd actual a comprobar
	int	sockConn; //socket de conexión entre servidor y cliente
	while (1)
	{
		std::cout << "-----------------------VUELTA--------------------------" << std::endl;
		ret = kevent(kq, NULL, 0, evList, 100, NULL); //devuelve el número de eventos triggereados
		if (ret == -1)
			return (-1);
		std::cout << "Número de eventos triggereados: " << ret << std::endl;
		for (int i = 0; i < ret; i++)
		{	
			currFd = (int)evList[i].ident;
			std::cout << "Evento " << i << ": " << currFd << std::endl;
			if (evList[i].flags & EV_ERROR)
			{
				std::cout << "Event error: " << strerror(evList[i].data) << std::endl; 
				close(sockConn);
			}
			/*else if (evList[i].flags & EV_EOF) {
				std::cout << "Client has disconnected" << std::endl;
				close(currFd);
			}*/
			else if (currFd == listenSocket) //si fd actual es el del socket de escucha, aceptamos nueva conexión y la leemos
			{
				std::cout << "Fd es socket de escucha" << std::endl;
				struct sockaddr_storage addr;
                socklen_t socklen = sizeof(addr);
				// Escuchar nuevo socket
                sockConn = accept(currFd, (struct sockaddr *)&addr, &socklen);
                //sobreescribir evento a monitorear
                EV_SET(&evSet, sockConn, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, NULL);
				//EV_SET(&evSet, sockConn, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, NULL);
                kevent(kq, &evSet, 1, NULL, 0, NULL); //lo añadimos a lista de eventos a monitorear
			}
			else if (evList[i].filter == EVFILT_READ)
			{
				std::cout << "Listo para leer" << std::endl;
				handle_connection(currFd);
				close(currFd);
			}
		}
	}
	close(kq);
}
