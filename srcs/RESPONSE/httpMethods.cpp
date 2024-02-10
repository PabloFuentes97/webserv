#include "../../includes/webserv.hpp"

static std::string	getRedir(struct client *client, itmap &redir)
{
	std::string redirs[] = {"postdir", "alias", "root", "cgi_pass"};
	std::string	&loc = client->loc->context._name;
	std::string file;
	if (client->request.url[client->request.url.length() -1] != '/')
	{
		int	locLen = client->loc->context._args[0].length();
		if (locLen > 1)
			locLen++;
		file = client->request.url.substr(locLen, client->request.url.length() - locLen + 1);
	}
	std::string path;
	size_t		i;

	for (i = 0; i < 4; i++)
	{
		if (redir->first == redirs[i])
		{
			switch (i)
			{
				case 0 : { path = redir->second; break ;}
				case 1 : { path = redir->second + file; break ;}
				case 2 : { path = redir->second + loc + file; break ;}
				case 3 : { path = redir->second + file; break ;}
			}
			break ;
		}
	}
	if (i == 4)
		throw (NOT_FOUND);
	return (path);
}

std::string	getPathFileRequest(client *client, std::vector<std::string>	&redirs)
{
	std::string	pathFile;
	int	locLen = client->loc->context._args[0].length();
	std::string file;
	
	itmap itm;
	size_t i;
	for (i = 0; i < redirs.size(); i++)
	{
		itm = client->loc->context._dirs.find(redirs[i]);
		if (itm != client->loc->context._dirs.end())
			break ;
	}
	if (i == redirs.size())
		throw (NOT_FOUND);
	if (client->request.url[client->request.url.length() -1] == '/')
		file = client->request.url.substr(locLen, client->request.url.length() - locLen + 1);
	pathFile = getRedir(client, itm);
	return (pathFile);
}

void	postMethod(client *client)
{
	if (!client->request.bufLen)
		throw (BAD_REQUEST);
	std::string filePath;
	std::vector<std::string>	redirs;
	redirs.push_back("postdir");
	filePath = getPathFileRequest(client, redirs);
	std::cout << "\033[0;33mPATH IN DIRECTORY: " << filePath << "\033[0m" << std::endl;
	itmap itm;
	itm = client->request.headers.find("Content-Type");
	if (itm == client->request.headers.end())
		throw (BAD_REQUEST);
	if (itm->second == "application/x-www-form-urlencoded\r" || itm->second == "application/x-www-form-urlencoded")
		postUrlEncoded(filePath, client->request.buf.c_str(), client->request.bufLen);
	else if (itm->second == "multipart/form-data")
		callMultiPart(client, filePath);
	else if (itm->second == "text/plain\r" || itm->second == "text/plain")
		postText(filePath, *client);
	else
		throw (BAD_REQUEST);
	client->request.status = 201;
	client->response.response = "HTTP/1.1 201 Created\r\n\r\n";
	std::cout << std::endl << "\033[0;32mRESPONSE HEADER IS: " << client->response.response << "\033[0m" << std::endl;
	client->response.response += "<html><body>File created at " + filePath + "\n</body>\n</html>\n";
}

static void	deleteMethod(client *client)
{
	struct stat	st;
	std::string filePath;
	std::vector<std::string>	redirs;
	redirs.push_back("alias");
	redirs.push_back("root");
	filePath = getPathFileRequest(client, redirs);
	std::cout << "\033[0;95mPATH IN DIRECTORY: " << filePath << "\033[0m" << std::endl;
	if (stat(filePath.c_str(), &st) == 0 && st.st_mode & S_IFDIR)
		throw (BAD_REQUEST);
	else
		if (remove(filePath.c_str()) < 0)
			throw(INTERNAL_SERVER_ERROR);
	client->request.status = 200;
	client->response.response = "HTTP/1.1 200 OK\r\n\r\n<html><body><h1>File deleted.</h1>\n</body>\n</html>\n";
	std::cout << std::endl << "\033[0;32mRESPONSE HEADER IS: " << client->response.response << "\033[0m" << std::endl;
}

static void	httpRedirect(client *client)
{
	std::string response;
	std::string	*redir = getMultiMapValue(client->loc->context._dirs, "redirect");
	if (!redir)
		return ;
	client->request.status = MOVED_PERMANENTLY;
	std::string http("http://");
	std::string	host = *(getMultiMapValue(client->request.headers, "Host"));
	host = host.substr(0, host.length() - 1);
	std::string file = client->request.url.substr(client->loc->context._args[0].length() + 1,
							client->request.url.length() - client->loc->context._args[0].length());
	std::string body;
	std::string redirect = "Location: " + http + host + *redir + file;
	response = "HTTP/1.1 301 Moved Permanently\r\n" + redirect + "\r\n\r\n";
	client->response.response = response;
	std::cout << std::endl << "\033[0;32mRESPONSE HEADER IS: " << client->response.response << "\033[0m" << std::endl;
	client->state = 3;
}

static void	checkMethods(client *client)
{
	std::string httpMethods[] = {"GET", "POST", "DELETE", "PUT", "HEAD", "CONNECT", "OPTIONS", "TRACE", "PATCH"};

	size_t i = 0;
	for (; i < 9; i++)
	{
		if (client->request.method == httpMethods[i])
			break ;
	}
	if (i == 9)
		throw (BAD_REQUEST);
	if (i >= 3)
		throw (NOT_IMPLEMENTED);
	if (!isInMultiMapKey(client->loc->context._dirs, "methods"))
		return ;
	if (isInMultiMapValue(client->loc->context._dirs, "methods", client->request.method))
	{
		if (client->request.method == "DELETE" && client->request.cgi == true)
			throw (METHOD_NOT_ALLOWED);
		return ;
	}
	throw (METHOD_NOT_ALLOWED);
}

static bool	checkBodySize(client *client)
{
	std::string	*bodySize = getMultiMapValue(client->loc->context._dirs, "limit_body_size");
	if (!bodySize)
		return (true);
	size_t	bodySizeInt = atoi((*bodySize).c_str());
	if (bodySizeInt < client->request.bufLen)
		return (false);
	return (true);
}

void ResponseToMethod(client *client)
{
	client->loc = matchLocation(client);
	if (!client->loc)
		throw (NOT_FOUND);
	std::cout << "\033[0;33mLOCATION: " << client->loc->context._args[0] << "\033[0m" << std::endl;
	if (checkBodySize(client) == false)
		throw (PAYLOAD_TOO_LARGE);
	checkMethods(client);
	if (isInMultiMapKey(client->loc->context._dirs, "redirect"))
	{
		httpRedirect(client);
		return ;
	}
	if (client->request.cgi)
	{
		CGIForward(client);
		client->state = 3;
		return ;
	}
	switch(client->request.method_int)
	{
		case HttpRequest::GET : {getMethod(client); break ;}
		case HttpRequest::POST : {postMethod(client); break ;}
		case HttpRequest::DELETE : {deleteMethod(client); break ;}
	}
	client->state = 3;
}