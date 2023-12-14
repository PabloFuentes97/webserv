#include "webserv.hpp"

HttpRequest loadRequest(char *buffer) {
	
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

	std::cout << std::endl << "METHOD:" << currentRequest.method << std::endl;
	std::cout << std::endl << "URL:" << currentRequest.url << std::endl;

	//Montar los headers
	while (std::getline(bufferFile, line)) {
        //Montar el mapa
        size_t pos = line.find(':');
        if (pos != std::string::npos) {

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
    
            currentRequest.headers[key] = value;
        }
    }
	
	return (currentRequest);
}

class	errorExcept : public std::exception
{
	virtual const char *what() const throw()
	{
		return ("Error");
	}
};
int	getKeyPos(std::vector<std::pair<std::string, std::vector<std::string> > > &map, std::string &search)
{
	for (int i = 0; i < map.size(); i++) //aqui se podría llamar a getValue, hacerlo busqueda binaria, o hacer un map
	{
		std::cout << "Map key: " << map[i].first << std::endl;
		if (map[i].first == search)
			return (i);
	}
	return (-1);
}

std::string	getPathFileRequest(bTreeNode *location, HttpRequest	&request)
{
	//primero busca una location que haga match con la URL pasada - tiene que buscar de más especifico a más general;
	//es la url la que tiene que compararse con la location - compara en base al numero de caracteres de la location
	//por ello si hay una location "/" hará match con cualquier URL pasada

	//si ha encontrado la location, tiene que buscar un root o alias
	//root -> path final de fichero a buscar = root + location + resto de URL (la parte final, restante que no coincide con la location)
	//alias -> path final de fichero a buscar = alias + resto de URL
	std::cout << "Location es: " << location->contextArgs[0] << std::endl;
	std::string	keys[] = {"alias", "root"};
	int	keysIndex;
	int	locIndex;
	std::string	pathFile;

	for (keysIndex = 0; keysIndex < 2; keysIndex++) //esto rehacerlo para que llame a otra funcion - rehacer getValue
	{
		std::cout << "Key tipo: " << keys[keysIndex] << std::endl;
		locIndex = getKeyPos(location->directives, keys[keysIndex]);
		if (locIndex != -1)
		{
			std::cout << "Encontró la key" << std::endl;
			break ;
		}
	}
	std::cout << "Key tipo encontrada: " << keys[keysIndex] << std::endl;
	std::string	&key = location->directives[locIndex].first;
	std::string	&value = location->directives[locIndex].second[0];
	int	locLen = location->contextArgs[0].length();
	std::string	fileDir = request.url.substr(locLen, locLen - value.length());
	std::cout << "URL sin la key, el resto: " << fileDir << std::endl;
	//coger path absoluto
	char	buf[1000];
	std::string absPath = getcwd(buf, 1000);
	std::string filePath;
	switch (keysIndex)
	{
		case 0: { std::cout << "Es alias: " << std::endl; filePath = absPath + value + fileDir; break ;} //alias
		case 1: { std::cout << "Es root: " << std::endl; filePath = absPath + value + location->contextArgs[0] + fileDir ; break ;} //root
	}
	std::cout << "filePath: " << filePath << std::endl;
	return (filePath);
}

std::string	getErrorPath(int error)
{
	char		buf[1000];
	std::string	absPath = getcwd(buf, 1000);
	std::string	errorPath;
	switch (error)
	{
		case 403: {errorPath = absPath + "/errors" + "/error403.html" ; break ; }
		case 404: {errorPath = absPath + "/errors" + "/error404.html" ; break ; }
	}
	return (errorPath);
}

std::string getRequestedFile(bTreeNode	*server, HttpRequest *currentRequest) {

	std::cout << "URL: " << currentRequest->url << std::endl;
	std::cout << "Entrar en findLocation" << std::endl;
	bTreeNode	*loc = findLocation(server, currentRequest->url);
	std::cout << "Hizo findLocation" << std::endl;
	std::string	filePath;
	std::string path;
	if (!loc) {
		currentRequest->status = 404;
		std::cout << "No encontró loc" << std::endl;
		throw (errorExcept());
	}
	filePath = getPathFileRequest(loc, *currentRequest);
	std::cout << "URL FOR REQUEST IS: " << filePath << std::endl;
	if (access(filePath.c_str(), F_OK) != 0)
		currentRequest->status = 404;
	else if (access(filePath.c_str(), R_OK) != 0)
		currentRequest->status = 403;
	else
		currentRequest->status = 200;
	if (currentRequest->status != 200)
	{
		std::cout << "filePath está mal" << std::endl;
		throw (errorExcept());
	}
	//si es un script (terminación) habrá q redirigir a CGI (ejecutar en un hijo);
	/*struct stat info;
	stat(filePath.c_str(), &info);
	if (S_ISDIR(info.st_mode) != 0)
		filePath = path + "/directory.html"; */
	std::cout << std::endl << "FILEPATH IS:" << filePath << std::endl; 
	return filePath; 
}

std::string getResponseBody(std::string fileToReturn) {

	std::ifstream file (fileToReturn);
	std::string fileLine;
	
    if (!file.is_open()) {
        std::cerr << "File error" << std::endl;
        exit (1); }
	std::cout << "Lee bien el fichero para enviar la respuesta" << std::endl;
	char c;
	while (file.get(c))
		fileLine.push_back(c);
	file.close();
	//std::cout << std::endl << "RESPONSE BODY IS: " << fileLine << std::endl;

	return fileLine;
}

std::string	getStatus(int status) {

	switch (status) {
		case 404:
			return "404 Not Found";
		case 403:
			return "403 Forbidden";
		default:
			return "200 OK";
	}
}

std::string getResponseFirstLine(HttpRequest currentRequest, std::string body) {

	std::string line = "HTTP/1.1 ";
	line.append(getStatus(currentRequest.status));
	line.append("\r\n");
	if (!body.empty()) {
		line.append("Content-Length: ");
		line.append(std::to_string((body).size()));
		line.append("\r\n");
	}
	line.append("Connection: close");
	line.append("\r\n\r\n");
	std::cout << std::endl << "RESPONSE HEADER IS: " << line << std::endl;

	return line;
}

std::string GetResponse(bTreeNode	*server, HttpRequest *request) {
	
	//bTreeNode	*
	std::string fileToReturn;
	try {
		fileToReturn = getRequestedFile(server, request);
	}
	catch(std::exception &e)
	{
		std::cout << e.what() << std::endl;
		fileToReturn = getErrorPath(request->status);
	}
	std::cout << "FILETORETURN: " << fileToReturn << std::endl;
	HttpResponse Response;
	// 
	// if (fileToReturn.substr(fileToReturn.find('.')) == ".php")
	// 	Response.body = getCgi(fileToReturn);
	// else
	Response.body = getResponseBody(fileToReturn);
	Response.firstLine = getResponseFirstLine(*request, Response.body);
	std::string finalRequest = Response.firstLine + Response.body;
	
	return finalRequest;
}

std::string ResponseToMethod(bTreeNode	*server, HttpRequest *request) {
	
	std::string response = "";
	if (request->method == "GET")
		response = GetResponse(server, request);
	else if (request->method == "POST")
		response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";;
	return response;
}