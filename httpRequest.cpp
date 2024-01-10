#include "webserv.hpp"

void	loadRequest(HttpRequest *request) {
	std::cout << "~~~~LOAD REQUEST STAGE~~~~" << std::endl;
	std::string line;
	std::string	header;
	for (int i = 0; i < request->buf_struct.filled; i++)
		header += request->buf_struct.ptr[i];
	std::cout << "Construyó el string" << std::endl;
	std::istringstream bufferFile(header.c_str());
	std::cout << "Creó el istringstream" << std::endl;
	//Coger la primera línea
	std::getline(bufferFile, line);
	//Sacar: método, url (y versión)
	size_t methodLength = line.find(' ');
    size_t urlLength = line.find(' ', methodLength + 1);
    if (methodLength != std::string::npos && urlLength != std::string::npos) {
        request->method = line.substr(0, methodLength);
        request->url = line.substr(methodLength + 1, urlLength - methodLength - 1);
    }
	else
	{
    	perror("invalid HTTP request");
		exit(0);
    }
	std::cout << std::endl << "METHOD: " << request->method << std::endl;
	std::cout << std::endl << "URL: " << request->url << std::endl;
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
			request->headers.insert(std::pair<std::string, std::string>(tokenKey, tokenValue));
		}
    }
	std::cout << "-------------------IMPRIMO MAPA-------------------\n\n\n";
	typedef std::multimap<std::string, std::string>::iterator	itm;
	for (itm b = request->headers.begin(), e = request->headers.end(); b != e; b++)
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

int	readHeader(struct client *client)
{
	
}

int	readEvent(struct client *client)
{
	std::cout << "---READ EVENT---" << std::endl;
	std::cout << "FD del cliente: " << client->fd << std::endl;

	//LEO EL PAQUETE QUE HA LLEGADO
    char buf[BUF_SIZE + 1];
	memset(buf, 0, sizeof(BUF_SIZE));
	int	bytes_read = recv(client->fd, buf, BUF_SIZE, MSG_DONTWAIT);
	std::cout << "Bytes read: " << bytes_read << std::endl;
	std::cout << "Buf leído: " << buf << std::endl;
	if (bytes_read == -1) {
		perror("recv");
		return (0);
	}
	if (bytes_read == 0) {
		/*std::cout << "[[CLOSE]]" << std::endl;
		close(cli->ident);*/
		return (1);
	}
	buf[bytes_read] = '\0';

	//CONCATENAR LO LEÍDO AL BUFFER ANTERIOR SI HABÍA ALGO
	std::cout << "Concatear buf" << std::endl;
	charptr_n	add_buff(buf, bytes_read);
	client->request.buf_struct += add_buff;

	std::cout << "Buffer total hasta ahora: \n";
	//std::cout << client->request.buf_struct.ptr << std::endl;
	1 << client->request.buf_struct;
	std::cout << "Longitud total del buffer concatenado: " << client->request.buf_struct.filled << std::endl;

	//BUSCAR SI EN EL BUFFER YA ESTÁ EL FINAL DEL HEADER
	char	*lim = strstr(client->request.buf_struct.ptr, "\r\n\r\n");
	if (client->state == 0 && lim)
	{
		std::cout << "Leyó todo el header" << std::endl;
		if (!*(lim + 4)) //NO HAY NADA MÁS DESPUÉS DEL HEADER
		{
			std::cout << "Solo leyó el header, no hay body" << std::endl;
			loadRequest(&client->request);
		}
		else //HACER SUBSTRING DEL HEADER Y LO QUE HA LEÍDO DEL BODY
		{
			charptr_n 	header;
			charptr_n	res;
			std::cout << "Guardar el resto" << std::endl;
			//HEADER
			header = subcharptr(client->request.buf_struct, 0, (lim + 4) - client->request.buf_struct.ptr);
			std::cout << "Guardó bien el header" << std::endl;
			std::cout << "Header: ";
			1 << header;
			std::cout << std::endl;
			//RESTO
			res = subcharptr(client->request.buf_struct, client->request.buf_struct.ptr - (lim + 4),
				client->request.buf_struct.filled - ((lim + 4) - client->request.buf));
			std::cout << "Guardó bien el resto" << std::endl;
			std::cout << "Resto: ";
			1 << res;
			std::cout << std::endl;
			client->request.buf_struct = header;
			loadRequest(&client->request);
			client->request.buf_struct = res;
		}
		client->state = 1;
		std::cout << "Pasa a estado 1" << std::endl;
	}
	if (client->state == 1 && client->request.buf_struct.state != charptr_n::EMPTY)
	{
		std::cout << "Tiene que leer el body" << std::endl;
		typedef std::multimap<std::string, std::string>::iterator itm;
		size_t	bodyLen;
		itm	it = client->request.headers.find("Content-Length");
		if (it != client->request.headers.end())
		{
			std::pair<itm, itm>	keyVal = client->request.headers.equal_range("Content-Length");
			bodyLen = atoi(keyVal.first->second.c_str());
		}
		else
		{
			std::cout << "No hay content-length" << std::endl;
			client->state = 2;
			std::cout << "Pasa a estado 2" << std::endl;
		}
		std::cout << "Longitud de buffer total leído: " << client->request.buf_struct.filled << " y content-length: " << bodyLen << std::endl;
		if (client->request.buf_struct.filled == bodyLen) //+1 por salto de línea final del body
		{
			std::cout << "Leyó ya todo el body" << std::endl;
			std::cout << "BODY: " << std::endl;
			1 << client->request.buf_struct;
			std::cout << std::endl;
			std::cout << "Pasa a estado 2" << std::endl;
			client->state = 2;
		}		
	}
	return (1);
}



/*int	readEvent(struct kevent *cli)
{
	std::cout << "---READ EVENT---" << std::endl;
	std::cout << "FD del cliente: " << cli->ident << std::endl;
	if (cli->flags & EV_EOF)
		std::cout << "flag es EV_EOF" << std::endl;
    // Read bytes from socket
    char buf[BUF_SIZE + 1];
	memset(buf, 0, sizeof(BUF_SIZE));
	int	bytes_read = recv(cli->ident, buf, BUF_SIZE, MSG_DONTWAIT);

	std::cout << "Bytes read: " << bytes_read << std::endl;
	//std::cout << "DATA (AFTER): " << client->data << std::endl;
	client	*clientCast = (client *)cli->udata;
	if (bytes_read == -1) {
		perror("recv");
	}
	if (bytes_read == 0) {
		std::cout << "[[CLOSE]]" << std::endl;
		close(cli->ident);
		return (1);
	}
	buf[bytes_read] = '\0';
	clientCast->request.buf.append(buf);
	//std::cout << "BUF: " << buf << std::endl;
	BUSCAR QUE LO LEÍDO CONTIENE EL FINAL DEL HEADER HTTP, MARCA QUE LO SIGUIENTE DEBERÍA SER EL BODY
	 - DIVIDIR LO QUE QUEDA DESPUÉS EN OTRA VARIABLE BODY COMO EN EL GETNEXTLINE
	std::cout << "Buffer total hasta ahora: " << clientCast->request.buf << std::endl;
	if (clientCast->state == 0 && strstr(buf, "\r\n\r\n"))
	{
		std::string	res;
		std::cout << "Leyó todo el header" << std::endl;
		int	endPos = clientCast->request.buf.find("\r\n\r\n");
		std::cout << "endPos: " << endPos + 4 << " , length: " << clientCast->request.buf.length() << std::endl;		
		if (endPos + 4 == clientCast->request.buf.length())
			std::cout << "Solo leyó el header, no hay body" << std::endl;
		else
		{
			std::cout << "Guardo el resto" << std::endl;
			res = clientCast->request.buf.substr(endPos + 6,
						endPos + 4 - clientCast->request.buf.length() + 1);
			std::cout << "Guardó bien el resto" << std::endl;
			clientCast->request.buf = clientCast->request.buf.substr(0, endPos + 1);
			std::cout << "Header: " << clientCast->request.buf << std::endl;
			std::cout << "Resto: " << res << std::endl;
		}
		//std::cout << "FULL BUFFER IS: " << clientCast->request.headerBuf << std::endl;
		loadRequest(&clientCast->request);
		clientCast->request.buf.clear();
		if (!res.empty())
			clientCast->request.buf = res;
		clientCast->state = 1;
		std::cout << "Pasa a estado 1" << std::endl;
	}
	if (clientCast->state == 1)
	{
		std::cout << "Tiene que leer el body" << std::endl;
		typedef std::multimap<std::string, std::string>::iterator itm;
		itm	it = clientCast->request.headers.find("Content-Length");
		if (it != clientCast->request.headers.end())
		{
			std::pair<itm, itm>	keyVal = clientCast->request.headers.equal_range("Content-Length");
			std::cout << "Content-Length: " << keyVal.first->first << std::endl;
			int	contLen = atoi(keyVal.first->first.c_str());
			if (contLen >= clientCast->request.buf.length())
				clientCast->state = 2;
		}
		else
		{
			std::cout << "No hay content-length" << std::endl;
		}
		clientCast->state = 2;
		//std::cout << "BODYBUFFER: " << std::endl << clientCast->request.bodyBuf << std::endl;
	}
	//std::cout << "BUF BIT: " << buf << std::endl;
	//std::cout << "fd is: " << client->ident << std::endl;
	
	//std::cout << "URL IS (PREV): " << Queue.clientArray[Queue.getPos(client->ident)].request.url << std::endl;
	
	EV_SET(&client_event[0], client->ident, EVFILT_WRITE, EV_ENABLE | EV_CLEAR, 0, 0, NULL);
	if (kevent(kq, &client_event[0], 1, NULL, 0, NULL) == -1)
		std::cerr << "kevent error" << std::endl;
	return (1);

}*/

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