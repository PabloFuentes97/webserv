#include "../../includes/webserv.hpp"

std::string	getPathFileRequest(bTreeNode *location, client *client)
{
	std::cout << "Location es: " << location->contextArgs[0] << std::endl;
	std::cout << "URL es: " << client->request.url << std::endl;
	std::string	pathFile;
	
	int	locLen = location->contextArgs[0].length();
	//coger path absoluto
	char	buf[1000];
	std::string absPath = getcwd(buf, 1000);
	std::string filePath;
	
	//BUSCAR SI TIENE CGI
	typedef std::multimap<std::string, std::string>::iterator itmap;
	itmap itm = location->directivesMap.find("cgi_pass");
	if (itm != location->directivesMap.end())
	{
		std::cout << "Encontró la key: " << itm->first << " | Valor: " << itm->second << std::endl;
		std::string	fileDir = client->request.url.substr(locLen, locLen - itm->second.length());
		std::cout << "fileDir: " << fileDir << std::endl;
		std::string	filePath = absPath + itm->second + fileDir;
		client->request.cgi = 1;
		std::cout << "filePath: " << filePath << std::endl;
		return (filePath);
	}
	switch(client->request.method_int)
	{
		case HttpRequest::POST : { 
			itm = location->directivesMap.find("postdir");
			if (itm == location->directivesMap.end())
			{
				std::cout << "No encontró la key de postdir" << std::endl;
				throw(404);
			}
			filePath = absPath + itm->second; break ;} 
		default : {
			std::string	keys[] = {"alias", "root"};
			int	i;
			for (i = 0; i < 2; i++) //esto rehacerlo para que llame a otra funcion - rehacer getValue
			{
				std::cout << "Key tipo: " << keys[i] << std::endl;
				itm = location->directivesMap.find(keys[i]);
				if (itm != location->directivesMap.end())
				{
					std::cout << "Encontró la key" << std::endl;
					break ;
				}
			}
			std::cout << "Key tipo encontrada: " << keys[i] << std::endl;
			std::string	fileDir = client->request.url.substr(locLen, locLen - itm->second.length());
			if (fileDir.empty())
			{
				typedef std::pair<itmap, itmap> itr;
				itr itk = location->directivesMap.equal_range("try_files");
				if (itk.first != location->directivesMap.end())
				{
					std::cout << "Encontró la key" << std::endl;
					break ;
				}
				for (itmap itb = itk.first, ite = itk.second; itb != ite; itb++)
				{
					if (!access(itb->second.c_str(), F_OK))
					{
						fileDir = itb->second;
						break ;
					}
				}
			}
			std::cout << "fileDir: " << fileDir << std::endl;
			switch (i)
			{
				case 0: { std::cout << "Alias: " << std::endl;
							filePath = absPath + itm->second + fileDir; break ;} //alias
				case 1: { std::cout << "Root: " << std::endl;
							filePath = absPath + itm->second + location->contextArgs[0] + fileDir ; break ;} //root
			}
		}
	}
	std::cout << "filePath: " << filePath << std::endl;
	return (filePath);
}

std::string	getRequestedFile(bTreeNode	*server, struct client *client)
{
	std::cout << "URL: " << client->request.url << std::endl;
	bTreeNode	*loc = findLocation(server, client->request.url);
	std::string	filePath;
	std::string path;
	if (!loc)
	{
		std::cout << "No encontró loc" << std::endl;
		client->request.status = 404;
		throw (404);
	}
	filePath = getPathFileRequest(loc, client);
	std::cout << std::endl << "FILEPATH IS:" << filePath << std::endl; 
	return (filePath); 
}

std::string	getMethod(bTreeNode	*server, client *client) {
	
	std::cout << "GET METHOD" << std::endl;
	std::string filePath;
	filePath = getRequestedFile(server, client);
	std::cout << "filePath: " << filePath << std::endl;
	if (access(filePath.c_str(), F_OK) != 0)
	{
		//client->request.status = 404;
		throw (404);
	}
	else if (access(filePath.c_str(), R_OK) != 0)
	{
		//client->request.status = 403;
		throw (403);
	}
	HttpResponse Response;
	client->request.status = 200;
	Response.firstLine = getResponseHeader(client->request, Response.body);
	if (client->request.cgi == 1)
		Response.body = CGIForward(filePath);
	else
		Response.body = getResponseBody(filePath);
	std::string finalRequest = Response.firstLine + Response.body;
	//std::cout << "FINALREQUEST: " << finalRequest << std::endl;
	return (finalRequest);
}

std::string	postMethod(bTreeNode *server, client *client)
{
	std::cout << "ESTOY EN POST" << std::endl;
	std::cout << "BODY REQUEST: " << client->request.buf << std::endl;
	std::string filePath;
	filePath = getRequestedFile(server, client);
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
		throw (400);
	//REHACER
	client->request.status = 201;
	return ("HTTP/1.1 201 Created\r\n\r\n");
}

std::string	deleteMethod(bTreeNode	*server, client *client)
{
	std::cout << "Delete method" << std::endl;
	
	struct stat	st;
	std::string filePath;
	filePath = getRequestedFile(server, client);
	std::cout << "filePath: " << filePath << std::endl;
	if (stat(filePath.c_str(), &st) == 0 && st.st_mode & S_IFDIR)
    {
		std::cout << "Es un directorio"  <<std::endl;
		if (rmdir(filePath.c_str()) < 0)
		{
			//client->request.status = 500;
			throw(500);
		}	
	}
	else
	{
		std::cout << "Es un fichero" << std::endl;
		if (remove(filePath.c_str()) < 0)
		{
			//client->request.status = 500;
			throw(500);
		}
	}
	client->request.status = 200;
	return ("HTTP/1.1 200 OK\r\n\r\n<html>	<body><h1>File deleted.</h1>\n  </body>\n</html>\n");
}

std::string ResponseToMethod(bTreeNode	*server, client *client)
{
	std::cout << "EN RESPONSE TO METHOD" << std::endl;
	std::string response;
	std::cout << "MÉTODO A EVALUAR: " << client->request.method_int << std::endl;
	try
	{
		//checkear cgi antes
		switch(client->request.method_int)
		{
			case HttpRequest::GET : {response = getMethod(server, client); break ;}
			case HttpRequest::POST : {response = postMethod(server, client); break ;}
			//case HttpRequest::PUT : {response = getMethod(server, client); break ;}
			case HttpRequest::DELETE : {response = deleteMethod(server, client); break ;}
		}
	}
	catch(int error)
	{
		client->request.status = error;
		response = getErrorResponse(server, client, error);
	}
	return (response);
}