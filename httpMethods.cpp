#include "webserv.hpp"

class	errorExcept : public std::exception
{
	virtual const char *what() const throw()
	{
		return ("Error");
	}
};

/*int	getKeyPos(std::vector<std::pair<std::string, std::vector<std::string> > > &map, std::string &search)
{
	for (int i = 0; i < map.size(); i++) //aqui se podría llamar a getValue, hacerlo busqueda binaria, o hacer un map
	{
		std::cout << "Map key: " << map[i].first << std::endl;
		if (map[i].first == search)
			return (i);
	}
	return (-1);
}*/

std::string	getPathFileRequest(bTreeNode *location, client *client, std::string &method) //cambiar para que haga diferentes cosas segun el metodo pasado
//añadir parametro method, como string o int - hacer un enum de methods
{
	//primero busca una location que haga match con la URL pasada - tiene que buscar de más especifico a más general;
	//es la url la que tiene que compararse con la location - compara en base al numero de caracteres de la location
	//por ello si hay una location "/" hará match con cualquier URL pasada

	//si ha encontrado la location, tiene que buscar un root o alias
	//root -> path final de fichero a buscar = root + location + resto de URL (la parte final, restante que no coincide con la location)
	//alias -> path final de fichero a buscar = alias + resto de URL
	std::cout << "Location es: " << location->contextArgs[0] << std::endl;
	std::cout << "URL es: " << client->request.url << std::endl;
	std::string	pathFile;
	
	int	locLen = location->contextArgs[0].length();
	//coger path absoluto
	char	buf[1000];
	std::string absPath = getcwd(buf, 1000);
	std::string filePath;
	std::multimap<std::string, std::string>::iterator itm;
	if (method == "POST")
	{
		itm = location->directivesMap.find("postdir");
		if (itm != location->directivesMap.end())
		{
			std::cout << "Encontró la key" << std::endl;
		}
		std::string	fileDir = client->request.url.substr(locLen, client->request.url.length() - locLen);
		std::cout << "URL sin la key, el resto: " << fileDir << std::endl;
		filePath = absPath + itm->second + fileDir;
	}
	else
	{
		std::string	keys[] = {"alias", "root"};
		int	keysIndex;
	
		for (keysIndex = 0; keysIndex < 2; keysIndex++) //esto rehacerlo para que llame a otra funcion - rehacer getValue
		{
			std::cout << "Key tipo: " << keys[keysIndex] << std::endl;
			itm = location->directivesMap.find(keys[keysIndex]);
			if (itm != location->directivesMap.end())
			{
				std::cout << "Encontró la key" << std::endl;
				break ;
			}
			/*locIndex = getKeyPos(location->directives, keys[keysIndex]);
			if (locIndex != -1)
			{
				std::cout << "Encontró la key" << std::endl;
				break ;
			}*/
		}
		std::cout << "Key tipo encontrada: " << keys[keysIndex] << std::endl;
		std::string	fileDir = client->request.url.substr(locLen, locLen - itm->second.length());
		std::cout << "URL sin la key, el resto: " << fileDir << std::endl;
		switch (keysIndex)
		{
			case 0: { std::cout << "Es alias: " << std::endl; filePath = absPath + itm->second + fileDir; break ;} //alias
			case 1: { std::cout << "Es root: " << std::endl; filePath = absPath + itm->second + location->contextArgs[0] + fileDir ; break ;} //root
		}
	}
	/*std::string	&key = location->directives[locIndex].first;
	std::string	&value = location->directives[locIndex].second[0];*/
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
		//client errors
		case 403: {errorPath = absPath + "/errors" + "/error403.html" ; break ; }
		case 404: {errorPath = absPath + "/errors" + "/error404.html" ; break ; }
		//server errors
		case 500: {errorPath = absPath + "/errors" + "/error500.html" ; break ; }
	}
	return (errorPath);
}

std::string	getRequestedFile(bTreeNode	*server, client *client) //cambiar esto - cada método gestiona de manera distinta el acceso a los ficheros
{

	std::cout << "URL: " << client->request.url << std::endl;
	bTreeNode	*loc = findLocation(server, client->request.url);
	std::string	filePath;
	std::string path;
	if (!loc) {
		std::cout << "No encontró loc" << std::endl;
		client->request.status = 404;
		throw (404);
	}
	filePath = getPathFileRequest(loc, client, client->request.method);
	std::cout << std::endl << "FILEPATH IS:" << filePath << std::endl; 
	return (filePath); 
}

std::string	getMethod(bTreeNode	*server, client *client) {
	
	std::cout << "GET METHOD" << std::endl;
	std::string filePath;
	try 
	{
		filePath = getRequestedFile(server, client);
	}
	catch(int status)
	{
		filePath = getErrorPath(status);
	}
	std::cout << "filePath: " << filePath << std::endl;
	if (access(filePath.c_str(), F_OK) != 0)
	{
		client->request.status = 404;
		throw (404);
	}
	else if (access(filePath.c_str(), R_OK) != 0)
	{
		client->request.status = 403;
		throw (403);
	}
	HttpResponse Response;
	// 
	// if (fileToReturn.substr(fileToReturn.find('.')) == ".php")
	// 	Response.body = getCgi(fileToReturn);
	// else
	client->request.status = 200;
	Response.firstLine = getResponseHeader(client->request, Response.body);
	Response.body = getResponseBody(filePath);
	std::string finalRequest = Response.firstLine + Response.body;
	//std::cout << "FINALREQUEST: " << finalRequest << std::endl;
	return (finalRequest);
}

std::string	postMethod(bTreeNode *server, client *client)
{
	std::cout << "ESTOY EN POST" << std::endl;
	std::cout << "BODY REQUEST: " << client->request.body << std::endl;
	std::string filePath;
	try 
	{
		filePath = getRequestedFile(server, client);
	}
	catch(int status)
	{
		filePath = getErrorPath(status);
	}
	if (access(filePath.c_str(), F_OK) == 0)
	{
		client->request.status = 404;
		throw (404);
	}
	std::string	postFile;
	typedef std::multimap<std::string, std::string>::iterator	itm;
	typedef	std::pair<itm, itm>	rangeKey;
	rangeKey itKey = client->request.headers.equal_range("Content-Disposition");
	std::cout << "Iterar sobre key Content-Disposition" << std::endl;
	for (itm b = itKey.first; b != itKey.second; b++)
	{
		std::cout << "Value: " << b->second << std::endl;
		if (b->second.compare(0, 9, "filename"))
		{
			std::stringstream	tokenValue(b->second);
			while (getline(tokenValue, postFile, '='));
			std::cout << "POSTFILE: " << postFile << std::endl;
			filePath += postFile; //aqui leer el file que especifica el body -> campo filename
			break ;
		}
	}
	std::cout << "FILEPATH FINAL: " << filePath << std::endl;
	int	fd = open(filePath.c_str(), O_CREAT | O_RDWR, 0666);
	if (fd < 0)
	{
		std::cout << "ERROR 500: falló fd" << std::endl;
		client->request.status = 500;
		throw (500);
	}
	std::string	fileBody = client->request.body;
	//std::string	fileBody = "prueba egege asasasa"; //sacarlo del body de la request
	size_t fileLength = strlen(fileBody.c_str());
	size_t bytes_sent = 0;
	//std::cout << "RESPONSE IS: " << finalRequest << std::endl;
	bytes_sent = write(fd, fileBody .c_str(), fileLength);
	close(fd);
	if (bytes_sent < 0)
	{
		throw(500);
		client->request.status = 500;
	}	
	else
		client->request.status = 200;
	
	return ("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n");
}

std::string	deleteMethod(bTreeNode	*server, client *client)
{
	std::cout << "Delete method" << std::endl;
	
	struct stat	st;
	std::string filePath;
	try 
	{
		filePath = getRequestedFile(server, client);
	}
	catch(int status)
	{
		client->request.status = status;
		throw(status);
	}
	std::cout << "filePath: " << filePath<< std::endl;
	if (stat(filePath.c_str(), &st) == 0 && st.st_mode & S_IFDIR)
    {
		std::cout << "Es un directorio"  <<std::endl;
		if (rmdir(filePath.c_str()) < 0)
		{
			client->request.status = 500;
			throw(500);
		}	
	}
	else
	{
		std::cout << "Es un fichero" << std::endl;
		if (remove(filePath.c_str()) < 0)
		{
			client->request.status = 500;
			throw(500);
		}
	}
	client->request.status = 200;
	return ("HTTP/1.1 200 OK\r\n\r\n<html>	<body><h1>File deleted.</h1>\n  </body>\n</html>\n");
}

std::string ResponseToMethod(bTreeNode	*server, client *client) {
	
	std::cout << "EN RESPONSE TO METHOD" << std::endl;
	std::string response;
	std::cout << "MÉTODO A EVALUAR: " << client->request.method << std::endl;
	//GET
	if (client->request.method == "GET")
	{
		try
		{
			response = getMethod(server, client);
		}
		catch(int error)
		{
			response = getErrorPath(error);
		}
	}
	//POST
	else if (client->request.method == "POST")
	{
		try
		{
			response = postMethod(server, client);
		}
		catch(int error)
		{
			std::cout << "Hay error: " << error << std::endl;
			response = getErrorPath(error);
		}
	}
	//DELETE	
	else if (client->request.method == "DELETE")
	{
		try
		{
			response = deleteMethod(server, client);
		}
		catch(int error)
		{
			response = getErrorPath(error);
		}
	}
	return (response);
}