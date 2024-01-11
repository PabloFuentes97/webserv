#include "webserv.hpp"

void	writeEvent(bTreeNode *server, clientQueue &Queue, int ident, struct kevent *client_event, int kq) {
	
	std::cout << "---WRITE EVENT---" << std::endl;
	std::string finalRequest  = ResponseToMethod(server, &(Queue.clientArray[Queue.getPos(ident)].request));
	
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

int	main(int argc, char **argv) {

	if (argc != 2) {
        std::cerr << "Format: ./webserv [configuration file]" << std::endl;
        return (1); }
    if (access(argv[1], R_OK) != 0) {
        std::cerr << "Inaccessible file" << std::endl;
        return 1; }
	struct kevent event[SOMAXCONN + 1];

	std::vector<int>	sockVec;
	int kq;
	int new_events;

	struct sockaddr_in	addr;
	struct sockaddr_in	client_addr;
	int					client_len;

	int new_sock;
	
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
	std::cout << "Hizo bucle" << std::endl;
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
	for (int i = 0; i < servers.size(); i++) {
		EV_SET(&server_event, sockVec[i], EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, NULL);
		//Registrar el evento
		if (kevent(kq, &server_event, 1, NULL, 0, NULL) == -1) {
    	    perror("kevent");
    	    exit(1);
   		}
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
			bool isServerSocket = false;
			if (event[i].ident <= sockVec.size() + 3)
			{
				std::cout << "Es socket de escucha: " << event[i].ident << std::endl;
				isServerSocket = true;
			}
			if (isServerSocket == true && (event[i].filter == EVFILT_READ)) {

				std::cout << "---ACCEPT EVENT---" << std::endl;
                //Montar cliente
                new_sock = accept(event[i].ident, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
				if (new_sock == -1) {
					std::cout << "ACCEPT ERROR" << std::endl;
					exit(0);
				}
				Queue.addClient(new_sock, event[i].ident);
				std::cout << "ADDED CLIENT. FD: " << event[i].ident << "SERVER ID: " << Queue.getServerId(new_sock) << std::endl;
				setNonBlocking(new_sock);
                // Nuevo evento al kqueue
				EV_SET(&client_event[1], new_sock, EVFILT_READ, EV_ADD | EV_CLEAR | EV_EOF, 0, 0, NULL); //hacer que udata sea puntero a cliente
				EV_SET(&client_event[0], new_sock, EVFILT_WRITE, EV_ADD | EV_DISABLE | EV_CLEAR, 0, 0, NULL);
				if (kevent(kq, client_event, 2, NULL, 0, NULL) < 0)
                    perror("kevent error");
			}
			else if (isServerSocket == false && (event[i].filter == EVFILT_WRITE))
				writeEvent(servers[Queue.getServerId(event[i].ident)], Queue, event[i].ident, client_event, kq);
			else if (isServerSocket == false  && (event[i].filter == EVFILT_READ))
				readEvent(Queue, &event[i], client_event, kq);
		}
	}
	return 0;
}

