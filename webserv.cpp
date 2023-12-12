#include "webserv.hpp"

void	writeEvent(clientQueue &Queue, int ident, struct kevent *client_event, int kq) {
	
	std::cout << "---WRITE EVENT---" << std::endl;
	std::string finalRequest  = ResponseToMethod(&(Queue.clientArray[Queue.getPos(ident)].request));
	
	size_t requestLength = strlen(finalRequest.c_str());
	size_t bytes_sent = 0;
	//std::cout << "RESPONSE IS: " << finalRequest << std::endl;
	while (bytes_sent < requestLength)
		bytes_sent += send(ident, finalRequest.c_str(), requestLength, MSG_DONTWAIT);
	Queue.clearRequest(ident);
	EV_SET(&client_event[0], ident, EVFILT_WRITE, EV_DISABLE | EV_CLEAR, 0, 0, NULL);
	if (kevent(kq, &client_event[0], 1, NULL, 0, NULL) == -1)
		std::cerr << "kevent error\n";
}

void	readEvent(clientQueue &Queue, struct kevent *client, struct kevent *client_event, int kq) {

	std::cout << "---READ EVENT---" << std::endl;
	std::cout << client->ident << std::endl;
	if (client->flags & EV_EOF)
		std::cout << "flag es EV_EOF" << std::endl;
    // Read bytes from socket
    char buf[BUF_SIZE + 1];
	std::string str;
	memset(buf, 0, sizeof(BUF_SIZE));
	//añadir manera de buclear el buffer
	int bytes_read = 5;
	while (bytes_read != -1) {
		//sleep(3); 
		memset(buf, 0, sizeof(BUF_SIZE));
		//std::cout << "DATA (BEFORE): " << client->data << std::endl;
		bytes_read = recv(client->ident, buf, BUF_SIZE, MSG_DONTWAIT);
		//std::cout << "DATA (AFTER): " << client->data << std::endl;
		if (bytes_read == -1) {
			perror("recv");
			break ;
		}
		if (bytes_read == 0) {
			std::cout << "[[CLOSE]]" << std::endl;
			close(client->ident);
			return;
		}
		buf[bytes_read] = '\0';
		//std::cout << "BUF BIT: " << buf << std::endl;
		str.append(buf);
	}
	std::cout << "FULL BUFFER IS: " << std::endl << str << std::endl;
	//std::cout << "fd is: " << client->ident << std::endl;
	Queue.clientArray[Queue.getPos(client->ident)].request = loadRequest((char *)str.c_str());
	std::cout << "URL IS (PREV): " << Queue.clientArray[Queue.getPos(client->ident)].request.url << std::endl;
	
	EV_SET(&client_event[0], client->ident, EVFILT_WRITE, EV_ENABLE | EV_CLEAR, 0, 0, NULL);
	if (kevent(kq, &client_event[0], 1, NULL, 0, NULL) == -1)
		std::cerr << "kevent error" << std::endl;

}

int	main() {

	struct kevent event[SOMAXCONN + 1];

	int sock;
	int kq;
	int new_events;

	struct sockaddr_in	addr;
	struct sockaddr_in	client_addr;
	int					client_len;

	int new_sock;

	//Socket del servidor
	sock = getServerSocket(&addr);
	bindAndListen(sock, &addr);

	//Montar el kqueue
	kq = kqueue();
	if (kq < 0) {
    	perror("kqueue()");
    	exit (1);
  	}

	//Declarar la clientQueue;
	clientQueue Queue;

	//Seter el evento para la kqueue
	struct kevent server_event;
	EV_SET(&server_event, sock, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, NULL);
	//Registrar el evento
	if (kevent(kq, &server_event, 1, NULL, 0, NULL) == -1) {
        perror("kevent");
        exit(1);
    }
	//Montar el cliente en el vector

	//Chequear para evento una vez cada ciclo


	for (;;) {
		new_events = kevent(kq, NULL, 0, event, SOMAXCONN + 1, NULL);
        if (new_events == -1) {
            perror("kevent");
            exit(1);
        }
		for (int i = 0; i < new_events; i++) {
			struct kevent client_event[2];
			std::cout << "========NEW EVENT======" << std::endl;
			//Socket de escucha
			if (event[i].ident == sock && (event[i].filter == EVFILT_READ)) {

				std::cout << "---ACCEPT EVENT---" << std::endl;
                //Montar cliente
                new_sock = accept(event[i].ident, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
				Queue.addClient(new_sock);
                if (new_sock == -1)
                    perror("Accept socket error");
				setNonBlocking(new_sock);

                // Nuevo evento al kqueue
				EV_SET(&client_event[1], new_sock, EVFILT_READ, EV_ADD | EV_CLEAR | EV_EOF, 0, 0, NULL);
				EV_SET(&client_event[0], new_sock, EVFILT_WRITE, EV_ADD | EV_DISABLE | EV_CLEAR, 0, 0, NULL);
				if (kevent(kq, client_event, 2, NULL, 0, NULL) < 0)
                    perror("kevent error");
			}

			else if (event[i].ident != sock && (event[i].filter == EVFILT_WRITE))
				writeEvent(Queue, event[i].ident, client_event, kq);

			else if (event[i].ident != sock && (event[i].filter == EVFILT_READ))
				readEvent(Queue, &event[i], client_event, kq);
		}
	}
	return 0;
}

