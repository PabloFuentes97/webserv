<<<<<<< HEAD
=======
#include "../../includes/webserv.hpp"

static std::string	getRedir(struct client *client, itmap &redir)
//pasar en vez del iterador con la clave y el valor ambos por separado
{
	std::cout << "En getRedir" << std::endl;
	std::string redirs[] = {"postdir", "alias", "root", "cgi_pass"};
	std::string	&loc = client->loc->context._name;
	std::string file;
	if (client->request.url[client->request.url.length() -1] != '/')
	{
		std::cout << "Es directorio, no coger nada de después" << std::endl;
		int	locLen = client->loc->context._args[0].length();
		file = client->request.url.substr(locLen, client->request.url.length() - locLen + 1);
		std::cout << "FILE A CONCATENAR: " << file << std::endl;
	}
	std::string path;
	size_t		i;

	for (i = 0; i < 4; i++)
	{
		if (redir->first == redirs[i])
		{
			std::cout << "Encontró: " << redirs[i] << std::endl;
			switch (i)
			{
				case 0 : { path = redir->second; break ;} //postdir
				case 1 : { path = redir->second + file; break ;} //alias
				case 2 : { path = redir->second + loc + file; break ;} //root
				case 3 : { path = redir->second + file; break ;} //cgi_pass
			}
			break ;
		}
	}
	if (i == 4)
		throw (404);
	return (path);
}

std::string	getPathFileRequest(client *client, std::vector<std::string>	&redirs)
//pasar como argumento un array de palabras claves donde buscar el directorio relativo
{
	std::cout << "En getPathFileRequest" << std::endl;
	std::cout << "Location es: " << client->loc->context._args[0] << std::endl;
	std::cout << "URL es: " << client->request.url << std::endl;
	std::string	pathFile;
	
	int	locLen = client->loc->context._args[0].length();
	std::string file;
	
	std::multimap<std::string, std::string>::iterator	itm;
	size_t i;
	for (i = 0; i < redirs.size(); i++) //esto cambiarlo para que llame a multimapKeysValue
	{
		std::cout << "Key tipo: " << redirs[i] << std::endl;
		itm = client->loc->context._dirs.find(redirs[i]);
		if (itm != client->loc->context._dirs.end())
		{
			std::cout << "Encontró la key" << std::endl;
			break ;
		}
	}
	if (i == redirs.size())
		throw (400);
	if (client->request.url[client->request.url.length() -1] == '/')
	{
		std::cout << "Es directorio, no coger nada de después" << std::endl;
		file = client->request.url.substr(locLen, client->request.url.length() - locLen + 1);
	}
	pathFile = getRedir(client, itm);
	std::cout << "PATH: " << pathFile << std::endl;
	return (pathFile);
}

void	getIndex(client *client)
{
	std::cout << "Es un directorio, usar index" << std::endl;
	itr itk = client->loc->context._dirs.equal_range("index");
	if (itk.first != client->loc->context._dirs.end())
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
			client->request.status = 200;
			return ;
		}
	}
	throw (404);
}

void	autoIndexListing(client *client)
{
	if (!isInMultiMapValue(client->loc->context._dirs, "autoindex", "on"))
		return ;

	std::vector<std::string>	redirsVec;
	redirsVec.push_back("alias");
	redirsVec.push_back("root");
	std::string	path = getPathFileRequest(client, redirsVec);
	std::cout << "Entro en autoindex" << std::endl;
	DIR	*dir = opendir(path.c_str());
	if (!dir)
		throw (400);
	dirent *elem = readdir(dir);
	std::string body;
	std::string	redirs[] = {"alias", "root"};
	body += "<html><body>";
	while (elem)
	{
		//std::cout << elem->d_name << std::endl;
		if (elem->d_name[0] != '.')
		{
			body += "<a href=\"http://";
			body += *(getMultiMapValue(client->request.headers, "Host"));
			body += "/" + getMultiMapValueKeys(client->loc->context._dirs, redirs, 2);
			body += elem->d_name;
			if (elem->d_type == DT_DIR)
				body += '/';
			body += "\">";
			body += elem->d_name;
			body += "\n</a>";
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
		if (getMultiMapValue(client->loc->context._dirs, dirs[i]))
		{
			f[i](client);
			if (!client->response.response.empty())
				return ;
		}
	}
	throw (400);
}

static void	pathIsFile(client *client, std::string &path)
{
	/*struct stat	st;
	if (stat(path.c_str(), &st) == -1)
		throw (404);
	if (st.st_mode & (~S_IRUSR))
		throw (403);*/

	if (access(path.c_str(), F_OK) != 0)
		throw (404);
	if (access(path.c_str(), R_OK) != 0)
		throw (403);

	HttpResponse Response;
	client->request.status = 200;
	Response.body = getResponseBody(path);
	Response.firstLine = getResponseHeader(client->request, Response.body);
	client->response.response = Response.firstLine + Response.body;
}

void	getMethod(client *client) {
	
	std::cout << "GET METHOD" << std::endl;
	std::vector<std::string>	redirs;
	redirs.push_back("alias");
	redirs.push_back("root");
	std::string	path = getPathFileRequest(client, redirs);
	struct stat	st;
	if (stat(path.c_str(), &st) == -1)
		throw (404);
	if (st.st_mode & S_IFDIR)
		pathIsDirectory(client);
	else
		pathIsFile(client, path);
}

void	postMethod(client *client)
{
	std::cout << "ESTOY EN POST" << std::endl;

	std::string filePath;
	std::vector<std::string>	redirs;
	redirs.push_back("postdir");
	filePath = getPathFileRequest(client, redirs);
	std::cout << "FILEPATH: " << filePath << std::endl;
	std::multimap<std::string, std::string>::iterator itm;
	itm = client->request.headers.find("Content-Type");
	if (itm == client->request.headers.end())
		throw (400);
	std::cout << "TIPO DE POST: " << itm->second << std::endl;
	if (itm->second == "application/x-www-form-urlencoded\r" || itm->second == "application/x-www-form-urlencoded")
		postUrlEncoded(filePath, client->request.buf.c_str(), client->request.bufLen);
	else if (itm->second == "multipart/form-data")
		callMultiPart(client, filePath);
	else if (itm->second == "text/plain\r" || itm->second == "text/plain")
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
	filePath = getPathFileRequest(client, redirs);
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

void	httpRedirect(client *client)
{
	std::string response;
	std::string	*redir = getMultiMapValue(client->loc->context._dirs, "redirect");
	if (!redir)
		throw (1);
	client->request.status = 301;
	std::string http("http://");
	std::string	host = *(getMultiMapValue(client->request.headers, "Host"));
	host = host.substr(0, host.length() - 1);
	std::string file = client->request.url.substr(client->loc->context._args[0].length(),
							client->request.url.length() - client->loc->context._args[0].length());
	std::string body;
	std::string redirect = "Location: " + http + host + *redir + file;
	response = "HTTP/1.1 301 Moved Permanently\r\n" + redirect + "\r\n\r\n";
	std::cout << "Response de redirect: " << response << std::endl;
	client->response.response = response;
	client->state = 3;
}

bool	checkMethods(client *client)
{
	if (!isInMultiMapKey(client->loc->context._dirs, "methods"))
		return (true);
	if (isInMultiMapValue(client->loc->context._dirs, "methods", client->request.method))
	{
		if (client->request.method == "DELETE" && client->request.cgi == true)
			return (false);
		return (true);
	}
	return (false);
}

bool	checkBodySize(client *client)
{
	std::cout << "Entro en bodySize" << std::endl;
	std::string	*bodySize = getMultiMapValue(client->loc->context._dirs, "limit_body_size");
	if (!bodySize)
	{
		std::cout << "No hay limit_bodysize" << std::endl;
		return (true);
	}
		
	size_t	bodySizeInt = atoi((*bodySize).c_str());
	if (bodySizeInt < client->request.bufLen)
	{
		std::cout << "bodySizeInt: " << bodySizeInt << " | bufLen: " << client->request.bufLen << std::endl;
		return (false);
	}
	std::cout << "Está bien" << std::endl;
	return (true);
}

void ResponseToMethod(client *client)
{
	std::cout << "EN RESPONSE TO METHOD" << std::endl;
	std::string response;
	client->loc = findLocation(client);
	if (!client->loc)
		throw (404);
	if (checkBodySize(client) == false)
		throw (413);
	if (checkMethods(client) == false)
		throw (405);

	//redirigir a otra location y enviarla por la respuesta
	if (isInMultiMapKey(client->loc->context._dirs, "redirect"))
	{
		httpRedirect(client);
		return ;
	}
	std::cout << "MÉTODO A EVALUAR " << client->request.method << " | URL: " << client->request.url << std::endl;
	if (client->request.cgi)
	{
		std::cout << "Entra en CGI" << std::endl;
		CGIForward(client);
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
>>>>>>> 8530eed3571046953b1c8862ce3a68c28294144d
