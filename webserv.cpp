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
	std::vector<client>	clients;
	//Seter el evento para la kqueue
	struct kevent server_event;
	for (int i = 0; i < servers.size(); i++) {
		EV_SET(&server_event, sockVec[i], EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, &clients[i]);
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
			client *clientPtr = (client *)event[i].udata;
			if (isServerSocket == true && (event[i].filter == EVFILT_READ)) {

				std::cout << "---ACCEPT EVENT---" << std::endl;
                //Montar cliente
                new_sock = accept(event[i].ident, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
				if (new_sock == -1) {
					std::cout << "ACCEPT ERROR" << std::endl;
					exit(0);
				}
				setNonBlocking(new_sock);
				client c = {new_sock, event[i].ident, 0};
				clients.push_back(c);
				Queue.addClient(new_sock, event[i].ident);
				//std::cout << "ADDED CLIENT. FD: " << event[i].ident << "SERVER ID: " << Queue.getServerId(new_sock) << std::endl;
				std::cout << "ADDED CLIENT. FD: " << c.fd << "SERVER ID: " << c.serverID << std::endl;
                // Nuevo evento al kqueue
				EV_SET(&client_event[1], new_sock, EVFILT_READ, EV_ADD | EV_CLEAR | EV_EOF, 0, 0, &clients.back());
				EV_SET(&client_event[0], new_sock, EVFILT_WRITE, EV_ADD | EV_DISABLE | EV_CLEAR, 0, 0, &clients.back());
				if (kevent(kq, client_event, 2, NULL, 0, NULL) < 0)
                    perror("kevent error");
			}
			else if (isServerSocket == false && (event[i].filter == EVFILT_WRITE) && clientPtr->state == 2)
			{
				writeEvent(servers[event[i].ident - 3], &event[i], client_event);
				//writeEvent(servers[Queue.getServerId(event[i].ident)], Queue, event[i].ident, client_event, kq);
				close(event[i].ident); //cerrar socket de conexion - se terminó
			}
			else if (isServerSocket == false && (event[i].filter == EVFILT_READ) && clientPtr->state < 2)
				readEvent(&event[i], client_event);
				//readEvent(Queue, &event[i], client_event, kq); //event es el evento que ha ocurrido; client_event es el tipo de evento a monitorear
		}
	}
	return 0;
}

