#include "webserv.hpp"

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

std::string	getPathFileRequest(bTreeNode *location, std::string	&url)
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
	std::string	fileDir = url.substr(locLen, locLen - value.length());
	std::cout << "URL sin la key, el resto: " << fileDir << std::endl;
	//coger path absoluto
	char	buf[1000];
	std::string absPath = getcwd(buf, 1000);
	std::cout << "Path absoluto: " << absPath << std::endl;
	std::string filePath;
	switch (keysIndex)
	{
		case 0: { std::cout << "Es alias: " << std::endl; filePath = absPath + value + fileDir; break ;} //alias
		case 1: { std::cout << "Es root: " << std::endl; filePath = absPath + value + location->contextArgs[0] + fileDir ; break ;} //root
	}
	std::cout << "filePath: " << filePath << std::endl;
	return (filePath);
}

std::string getRequestedFile(bTreeNode	*server, client *client) {

	std::cout << "URL: " << client->request.url << std::endl;
	std::cout << "Entrar en findLocation" << std::endl;
	bTreeNode	*loc = findLocation(server, client->request.url);
	std::cout << "Hizo findLocation" << std::endl;
	std::string	filePath;
	std::string path;
	if (!loc) {
		/* currentRequest->status = 404; */
		std::cout << "No encontró loc" << std::endl;
		throw (errorExcept());
	}
	filePath = getPathFileRequest(loc, client->request.url);
	std::cout << "URL FOR REQUEST IS: " << filePath << std::endl;

	 if (access(filePath.c_str(), F_OK) != 0)
		client->request.status = 404;
	else if (access(filePath.c_str(), R_OK) != 0)
		client->request.status = 403;
	else
		client->request.status = 200;
	if (client->request.status != 200)
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

int	GetMethod(bTreeNode	*server, client *client) 
{
	std::string	filePath;
	try {
		filePath = getRequestedFile(server, client);
		response = header
		if (get)
			response += body
	}
	catch(int status)
	{
		filePath = getErrorPath(status); // response = 
	}
	std::cout << "filePath: " << client->response << std::endl;

	HttpResponse Response;
	Response.body = getResponseBody(client->response);
	Response.firstLine = getResponseFirstLine(client->request, Response.body);
	client->response = Response.firstLine + Response.body;
	// 
	// if (fileToReturn.substr(fileToReturn.find('.')) == ".php")
	// 	Response.body = getCgi(fileToReturn);
	// else
}


/* 
std::string GetMethod(bTreeNode	*server, HttpRequest *request) {
	
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
} */

int	postMethod(bTreeNode *server, client *client)
{

	return (200);
}

int	deleteMethod(bTreeNode *server, client *client)
{
	struct stat	st;

	std::cout << "Delete method" << std::endl;
	std::string	file = getRequestedFile(server, client->request.url);
	if (access(file.c_str(), F_OK) < 0) //hacer una función generica de search o find y pasar una funcion como arg
	{
		std::cout << "NOZING BORRAO\n";
		return (404);
	}	
	
	if (stat(file.c_str(), &st) == 0 && !(st.st_mode & S_IFDIR))
    {
		std::cout << "borrao DIR\n";
		if (rmdir(file.c_str()) < 0)
			return (404);
	}
	else
	{
		std::cout << "borrao FILE\n";
		if (remove(file.c_str()) < 0)
			return (404);
	}
	char	response[] = "HTTP/1.1 200 OK\r\n\r\n<h1>File deleted.</h1>\n  </body>\n</html>\n";
	//write(socket, response, 114);
	return (200);
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

//Hacer respuestas dentro de los métodos
std::string ResponseToMethod(bTreeNode	*server, client	*client) {
	
	std::string response;
	if (client->request.method == "GET")
		response = GetMethod(server, request->url);
	else if (client->request.method == "POST")
		response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
	else if (client->request.method == "DELETE")
		response = deleteMethod(server, request->url);
	return response;
}
/* 
throw (int err)
switch (err)
	if err = 404
		response =  */