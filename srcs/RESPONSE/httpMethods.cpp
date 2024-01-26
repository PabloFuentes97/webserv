#include "../../includes/webserv.hpp"

std::string	getRedir(struct client *client, itmap &redir, std::string &file)
//pasar en vez del iterador con la clave y el valor ambos por separado
{
	std::cout << "En getRedir" << std::endl;
	std::string redirs[] = {"postdir", "alias", "root"};
	std::string	&loc = client->loc->contextArgs[0];
	//coger path absoluto
	char	buf[1000];
	std::string absPath = getcwd(buf, 1000);
	std::string path;
	size_t		i;

	for (i = 0; i < 3; i++)
	{
		if (redir->first == redirs[i])
		{
			std::cout << "Encontró: " << redirs[i] << std::endl;
			switch (i)
			{
				case 0 : { path = absPath + redir->second; break ;} //postdir
				case 1 : { path = absPath + redir->second + file; break ;} //alias
				case 2 : { path = absPath + redir->second + loc + file; break ;} //root
			}
			break ;
		}
	}
	if (i == 3)
		throw (404);
	return (path);
}

std::string	getPathFileRequest(client *client, std::vector<std::string>	&redirs)
//pasar como argumento un array de palabras claves donde buscar el directorio relativo
{
	std::cout << "En getPathFileRequest" << std::endl;
	bTreeNode	&loc = *(client->loc);
	std::cout << "Location es: " << loc.contextArgs[0] << std::endl;
	std::cout << "URL es: " << client->request.url << std::endl;
	std::string	pathFile;
	
	int	locLen = loc.contextArgs[0].length();
	//coger path absoluto
	char	buf[1000];
	std::string absPath = getcwd(buf, 1000);
	std::string file;
	
	std::multimap<std::string, std::string>::iterator	itm;
	size_t i;
	for (i = 0; i < redirs.size(); i++) //esto cambiarlo para que llame a multimapKeysValue
	{
		std::cout << "Key tipo: " << redirs[i] << std::endl;
		itm = loc.directivesMap.find(redirs[i]);
		if (itm != loc.directivesMap.end())
		{
			std::cout << "Encontró la key" << std::endl;
			break ;
		}
	}
	if (i == redirs.size())
		throw (400);
	//sacar la parte del directorio fuera
	/*if (client->request.url[client->request.url.size() - 1] == '/') //es un directorio
		pathFile = getIndex(client, absPath);*/
	//es un fichero
	file = client->request.url.substr(locLen, client->request.url.length() - locLen + 1);
	pathFile = getRedir(client, itm, file);
	return (pathFile);
}

std::string	getRequestedFile(struct client *client, std::vector<std::string> &redirs) //sacar location y pasarlo como argumento
{
	std::cout << "URL: " << client->request.url << std::endl;
	//bTreeNode	*loc = findLocation(server, client->request.url);
	std::string	filePath;
	std::string path;
	if (!client->loc)
	{
		std::cout << "No encontró loc" << std::endl;
		client->request.status = 404;
		throw (404);
	}
	filePath = getPathFileRequest(client, redirs);
	std::cout << std::endl << "FILEPATH IS:" << filePath << std::endl; 
	return (filePath); 
}

void	getIndex(client *client)
{
	std::cout << "Es un directorio, usar index" << std::endl;
	bTreeNode &loc = *(client->loc);
	itr itk = loc.directivesMap.equal_range("index");
	if (itk.first != loc.directivesMap.end())
		std::cout << "Encontró la key de index: " << std::endl;
	std::string	path;
	std::vector<std::string>	redirs;
	redirs.push_back("alias");
	redirs.push_back("root");
	for (itmap itb = itk.first, ite = itk.second; itb != ite; itb++)
	{
		path = getPathFileRequest(client, redirs) + itb->second;
		std::cout << "path de file index: " << path << std::endl;
		if (!access(path.c_str(), F_OK | R_OK))
		{
			std::cout << "Existe fichero " << std::endl;
			std::string body = getResponseBody(path);
			client->response.response = getResponseHeader(client->request, body) + body;
			return ;
		}
	}
	throw (404);
}

void	autoIndexListing(client *client)
{
	if (!isInMultiMapValue(client->loc->directivesMap, "autoindex", "on"))
		return ;
	std::cout << "Entro en autoindex" << std::endl;
	char	buf[1000];
	std::string absPath = getcwd(buf, 1000) + client->request.url;
	std::cout << "Abspath: " << absPath << std::endl;
	DIR	*dir = opendir(absPath.c_str());
	if (!dir)
		throw (400);
	dirent *elem = readdir(dir);
	std::string body;
	std::string	redirs[] = {"alias", "root"};
	body += "<html><body>";
	while (elem)
	{
		std::cout << elem->d_name << std::endl;
		if (elem->d_name[0] != '.')
		{
			//body += "<h1>";
			body += "<a href=\"http://";
			//body += '"';
			//body += "http://";
			//sustituir esto por una funcion que devuelva la primera key que encuentra en un array de prioridades
			//body += getRequestedFile(client, redirs);
			body += *(getMultiMapValue(client->request.headers, "Host"));
			body += getMultiMapValueKeys(client->loc->directivesMap, redirs, 2);
			body += elem->d_name;
			if (elem->d_type == DT_DIR)
				body += '/';
			body += "\">";
			body += elem->d_name;
			body += "\n</a>";
			//body += "</a>";
			//body += "</h1>";
		}
		elem = readdir(dir);
	}
	body += "</body></html>";
	closedir(dir);
	client->request.status = 200;
	client->response.response = getResponseHeader(client->request, body) + body;
}

static void pathIsDirectory(client *client)
{
	std::string dirs[] = {"autoindex", "index"};
	void (*f[])(struct client *client) = {autoIndexListing, getIndex}; //añadir getIndex al principio
	for (size_t i = 0; i < 2; i++)
	{
		if (getMultiMapValue(client->loc->directivesMap, dirs[i]))
		{
			f[i](client);
			if (!client->response.response.empty())
				return ;
		}
	}
	throw (400);
}

void	getMethod(client *client) {
	
	std::cout << "GET METHOD" << std::endl;

	if (client->request.url[client->request.url.size() - 1] == '/') //es un directorio
			pathIsDirectory(client);
	else
	{
		std::string 				filePath;
		std::vector<std::string>	redirs;
		redirs.push_back("alias");
		redirs.push_back("root");
		filePath = getRequestedFile(client, redirs);
		std::cout << "filePath: " << filePath << std::endl;
		if (access(filePath.c_str(), F_OK) != 0)
			throw (404);
		if (filePath[filePath.size() - 1] == '/')
			return (pathIsDirectory(client));
		if (access(filePath.c_str(), R_OK) != 0)
			throw (403);
		HttpResponse Response;
		client->request.status = 200;
		Response.body = getResponseBody(filePath);
		Response.firstLine = getResponseHeader(client->request, Response.body);
		client->response.response = Response.firstLine + Response.body;
	}
}

void	postMethod(client *client)
{
	std::cout << "ESTOY EN POST" << std::endl;
	//std::cout << "BODY REQUEST: " << client->request.buf << std::endl;
	std::string filePath;
	std::vector<std::string>	redirs;
	redirs.push_back("postdir");
	filePath = getRequestedFile(client, redirs);
	std::cout << "FILEPATH: " << filePath << std::endl;
	std::multimap<std::string, std::string>::iterator itm;
	itm = client->request.headers.find("Content-Type");
	if (itm != client->request.headers.end())
	{
		std::cout << "Encontró la key: " << itm->first << " , Value: " << itm->second << std::endl;
	}
	//DIFERENTES TIPOS DE POST: APPLICATION, MULTIPART-FORM, TEXT-PLAIN
	std::cout << "TIPO DE POST: " << itm->second << std::endl;
	if (itm->second == "application/x-www-form-urlencoded\r")
		postUrlEncoded(filePath, client->request.buf.c_str(), client->request.bufLen);
	else if (itm->second == "multipart/form-data")
		callMultiPart(client, filePath);
	else if (itm->second == "text/plain\r")
		postText(filePath, client->request.buf.c_str(), client->request.bufLen);
	else
	{
		std::cout << "Lanza 400" << std::endl;
		throw (400);
	}
	client->response.response = "HTTP/1.1 201 Created\r\n\r\n";
}

void	deleteMethod(client *client)
{
	struct stat	st;
	std::string filePath;
	std::vector<std::string>	redirs;
	redirs.push_back("alias");
	redirs.push_back("root");
	filePath = getRequestedFile(client, redirs);
	if (stat(filePath.c_str(), &st) == 0 && st.st_mode & S_IFDIR)
    {
		std::cout << "Es un directorio"  <<std::endl;
		if (rmdir(filePath.c_str()) < 0)
			throw(500);
	}
	else
	{
		std::cout << "Es un fichero" << std::endl;
		if (remove(filePath.c_str()) < 0)
			throw(500);
	}
	client->response.response = "HTTP/1.1 200 OK\r\n\r\n<html><body><h1>File deleted.</h1>\n</body>\n</html>\n";
}

std::string	httpRedirect(client *client)
{
	std::string response;
	std::string	*redir = getMultiMapValue(client->loc->directivesMap, "redirect");
	if (!redir)
		throw (1);
	client->request.status = 301;
	std::string http("http://");
	std::string	host = *(getMultiMapValue(client->request.headers, "Host"));
	host = host.substr(0, host.length() - 1);
	std::string file = client->request.url.substr(client->loc->contextArgs[0].length(),
							client->request.url.length() - client->loc->contextArgs[0].length());
	std::string body;
	std::string redirect = "Location: " + http + host + *redir + file;
	std::cout << "Host: " << host << std::endl;
	std::cout << "Redir: " << *redir << std::endl;
	std::cout << "File: " << file << std::endl;
	std::cout << "Body completo: " << body << std::endl;
	//response = getResponseHeader(client->request, body);
	response = "HTTP/1.1 301 Moved Permanently\r\n" + redirect + "\r\n\r\n";
	std::cout << "Response de redirect: " << response << std::endl;
	return (response);
}

bool	checkMethods(client *client)
{
	if (!isInMultiMapKey(client->loc->directivesMap, "methods"))
		return (true);
	if (isInMultiMapValue(client->loc->directivesMap, "methods", client->request.method))
	{
		if (client->request.method == "DELETE" && client->request.cgi == true)
			return (false);
		return (true);
	}
	return (false);
}

bool	checkBodySize(client *client, int bodyLen)
{
	std::string	bodySize = *getMultiMapValue(client->server->directivesMap, "limit_body_size");
	int	bodySizeInt = atoi(bodySize.c_str());
	if (bodySizeInt > bodyLen)
		return (false);
	return (true);
}

void ResponseToMethod(client *client)
{
	std::cout << "EN RESPONSE TO METHOD" << std::endl;
	std::string response;
	client->loc = findLocation(client);
	if (!client->loc)
		throw (404);
	if (!checkMethods(client))
		throw (405);
	//redirigir a otra location y enviarla por la respuesta
	if (isInMultiMapKey(client->loc->directivesMap, "redirect"))
	{
		client->response.response = httpRedirect(client);
		client->state = 3;
		return ;
	}
	std::cout << "MÉTODO PERMITIDO: " << client->request.method << std::endl;
	std::cout << "MÉTODO A EVALUAR: " << client->request.method_int << std::endl;

	if (client->request.cgi) {
		std::cout << "Entra en CGI" << std::endl;
		//CGI
	}
	else 
	{
		//pasar filepath como parámetro o montarlo sobre el cliente en vez d en cada método
		switch(client->request.method_int)
		{
			case HttpRequest::GET : {getMethod(client); break ;}
			case HttpRequest::POST : {postMethod(client); break ;}
			case HttpRequest::DELETE : {deleteMethod(client); break ;}
		}
	}
	client->state = 3;
	return ;
}