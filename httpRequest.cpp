#include "webserv.hpp"

void	loadRequest(HttpRequest *request) {
	
	HttpRequest currentRequest;
	std::string line;
	std::istringstream bufferFile(request->headerBuf.c_str());
	std::cout << "~~~~LOAD REQUEST STAGE~~~~" << std::endl;
	//Coger la primera línea
	std::getline(bufferFile, line);
	//Sacar: método, url (y versión)
	size_t methodLength = line.find(' ');
    size_t urlLength = line.find(' ', methodLength + 1);
    if (methodLength != std::string::npos && urlLength != std::string::npos) {
        currentRequest.method = line.substr(0, methodLength);
        currentRequest.url = line.substr(methodLength + 1, urlLength - methodLength - 1);
    } else {
        perror("invalid HTTP request");
		exit (0);
    }
	std::cout << std::endl << "METHOD: " << currentRequest.method << std::endl;
	std::cout << std::endl << "URL: " << currentRequest.url << std::endl;
	std::string	tokenKey;
	std::string	tokenValue;
	//Montar los headers
	while (std::getline(bufferFile, line)) {
        //Montar el mapa
		std::stringstream	streamLine(line);
		//std::cout << "Línea: " << line << std::endl;
        getline(streamLine, tokenKey, ':');
		//std::cout << "TokenKey: " << tokenKey << std::endl;
		while (getline(streamLine, tokenValue, ';'))
		{
			//std::cout << "TokenValue: " << tokenValue << std::endl;
			currentRequest.headers.insert(std::pair<std::string, std::string>(tokenKey, tokenValue));
		}
				
		/*VERSION DE ANDREA----------
		size_t pos = line.find(':');
        if (pos != std::string::npos) {

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
			std::cout << "Key: " << key << ", Value: " << value << std::endl;
            //currentRequest.headers[key] = value;
			currentRequest.headers.insert(std::pair<std::string, std::string>(key, value));
        }*/
    }
	std::cout << "-------------------IMPRIMO MAPA-------------------\n\n\n";
	typedef std::multimap<std::string, std::string>::iterator	itm;
	for (itm b = currentRequest.headers.begin(), e = currentRequest.headers.end(); b != e; b++)
	{
		std::cout << "Key: " << b->first << " | Value: " << b->second << std::endl;
	}
	std::cout << "\n\n-------------------TERMINA DE IMPRIMIR MAPA-------------------\n\n\n";
}

/*HttpRequest loadRequest(char *buffer) {
	
	HttpRequest currentRequest;
	std::string line;
	std::istringstream bufferFile(buffer);
	std::cout << "~~~~LOAD REQUEST STAGE~~~~" << std::endl;
	//Coger la primera línea
	std::getline(bufferFile, line);
	//Sacar: método, url (y versión)
	size_t methodLength = line.find(' ');
    size_t urlLength = line.find(' ', methodLength + 1);
    if (methodLength != std::string::npos && urlLength != std::string::npos) {
        currentRequest.method = line.substr(0, methodLength);
        currentRequest.url = line.substr(methodLength + 1, urlLength - methodLength - 1);
    } else {
        perror("invalid HTTP request");
		exit (0);
    }
	std::cout << std::endl << "METHOD: " << currentRequest.method << std::endl;
	std::cout << std::endl << "URL: " << currentRequest.url << std::endl;
	std::string	tokenKey;
	std::string	tokenValue;
	//Montar los headers
	while (std::getline(bufferFile, line)) {
        //Montar el mapa
		std::stringstream	streamLine(line);
		//std::cout << "Línea: " << line << std::endl;
        getline(streamLine, tokenKey, ':');
		//std::cout << "TokenKey: " << tokenKey << std::endl;
		while (getline(streamLine, tokenValue, ';'))
		{
			//std::cout << "TokenValue: " << tokenValue << std::endl;
			currentRequest.headers.insert(std::pair<std::string, std::string>(tokenKey, tokenValue));
		}
				
		VERSION DE ANDREA----------
		size_t pos = line.find(':');
        if (pos != std::string::npos) {

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
			std::cout << "Key: " << key << ", Value: " << value << std::endl;
            //currentRequest.headers[key] = value;
			currentRequest.headers.insert(std::pair<std::string, std::string>(key, value));
        }
    }
	std::cout << "-------------------IMPRIMO MAPA-------------------\n\n\n";
	typedef std::multimap<std::string, std::string>::iterator	itm;
	for (itm b = currentRequest.headers.begin(), e = currentRequest.headers.end(); b != e; b++)
	{
		std::cout << "Key: " << b->first << " | Value: " << b->second << std::endl;
	}
	std::cout << "\n\n-------------------TERMINA DE IMPRIMIR MAPA-------------------\n\n\n";
	return (currentRequest);
}

std::string ResponseToMethod(bTreeNode	*server, HttpRequest *request) {
	
	std::string response;
	if (request->method == "GET")
		response = GetResponse(server, request->url);
	else if (request->method == "POST")
		response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";;
	return response;
}*/

int	readEvent(struct kevent *cli, struct kevent *client_event)
{
	std::cout << "---READ EVENT---" << std::endl;
	std::cout << "FD del cliente: " << cli->ident << std::endl;
	if (cli->flags & EV_EOF)
		std::cout << "flag es EV_EOF" << std::endl;
    // Read bytes from socket
    char buf[BUF_SIZE + 1];
	memset(buf, 0, sizeof(BUF_SIZE));
	int	bytes_read;
	bytes_read = recv(cli->ident, buf, BUF_SIZE, MSG_DONTWAIT);
	std::cout << "Bytes read: " << bytes_read << std::endl;
	//std::cout << "DATA (AFTER): " << client->data << std::endl;
	if (bytes_read == -1) {
		perror("recv");
	}
	if (bytes_read == 0) {
		std::cout << "[[CLOSE]]" << std::endl;
		close(cli->ident);
		return (1);
	}
	buf[bytes_read] = '\0';
	//std::cout << "BUF: " << buf << std::endl;
	client	*clientCast = (client *)cli->udata;
	/*BUSCAR QUE LO LEÍDO CONTIENE EL FINAL DEL HEADER HTTP, MARCA QUE LO SIGUIENTE DEBERÍA SER EL BODY
	 - DIVIDIR LO QUE QUEDA DESPUÉS EN OTRA VARIABLE BODY COMO EN EL GETNEXTLINE*/
	if (clientCast->state == 0 && strstr(buf, "\r\n\r\n"))
	{
		std::cout << "Leyó todo el header" << std::endl;
		clientCast->request.headerBuf.append(buf);
		//std::cout << "FULL BUFFER IS: " << clientCast->request.headerBuf << std::endl;
		loadRequest(&clientCast->request);
		clientCast->state = 1;
		return (0);
	}
	if (clientCast->state == 1)
	{
		std::cout << "Lee el body" << std::endl;
		clientCast->request.bodyBuf.append(buf);
		//std::cout << "BODYBUFFER: " << std::endl << clientCast->request.bodyBuf << std::endl;
		return (0);
	}
	//std::cout << "BUF BIT: " << buf << std::endl;
	//std::cout << "fd is: " << client->ident << std::endl;
	
	//std::cout << "URL IS (PREV): " << Queue.clientArray[Queue.getPos(client->ident)].request.url << std::endl;
	
	/*EV_SET(&client_event[0], client->ident, EVFILT_WRITE, EV_ENABLE | EV_CLEAR, 0, 0, NULL);
	if (kevent(kq, &client_event[0], 1, NULL, 0, NULL) == -1)
		std::cerr << "kevent error" << std::endl;*/
	clientCast->state = 2;
	return (1);

}

/*void	readEvent(clientQueue &Queue, struct kevent *client, struct kevent *client_event, int kq)
{

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
		BUSCAR QUE LO LEÍDO CONTIENE EL FINAL DEL HEADER HTTP, MARCA QUE LO SIGUIENTE DEBERÍA SER EL BODY
		 - DIVIDIR LO QUE QUEDA DESPUÉS EN OTRA VARIABLE BODY COMO EN EL GETNEXTLINE
		if (str.find("\r\n\r\n "))
			break ;
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

}*/