#include "../../includes/webserv.hpp"

std::string	getStatus(int status)
{
	std::cout << "REQUEST STATUS: " << status << std::endl;
	switch (status) {
        case 201:
            return "201 Created";
		case 301:
			return "301 Moved Permanently";
        case 400:
            return "400 Bad Request"; //El formato de la request está mal
        case 403:
            return "403 Forbidden"; //Acceso no permitido
        case 404:
            return "404 Not Found"; //Archivo no encontrado
        case 405:
            return "405 Method Not Allowed"; //Método que no gestionamos (DELETE de CGI o cualquier otro)
        case 408:
            return "408 Request Timeout"; //Timeout
		case 409:
            return "409 Conflict";
        case 413:
            return "413 Payload Too Large"; //Request muy grande
        case 429:
            return "429 Too Many Request"; //Muchas
        case 500:
            return "500 Internal Server Error";
        case 501:
            return "501 Not Implemented";
        case 502:
            return "502 Bad Gateway";
        case 504:
            return "504 Gateway Timeout";
        default:
            return "200 OK";
    }
}

std::string	getErrorPath(struct client *client, int error)
{
	std::cout << "ERROR: " << error << std::endl;
	if (client->loc)
	{
		typedef std::multimap<std::string, std::string>::iterator itm;
		
		itm it = client->loc->context._dirs.find("error_files");
		if (it != client->loc->context._dirs.end())
		{
			std::pair<itm, itm> itr = client->loc->context._dirs.equal_range("error_files");
			for (itm ib = itr.first, ie = itr.second; ib != ie; ib++)
			{
				int	n = atoi(ib->second.c_str());
				if (n == error)
				{
					ie--;
					return (ie->second);
				}
			}
		}
	}
	return ("errors/error" + std::to_string(error) + ".html");
}

void	getErrorResponse(struct client *client, int error)
{
	std::string	resp;
	std::string	path;
	std::string body;
	path = getErrorPath(client, error);
	try
	{
		body = getResponseBody(path);
	}
	catch(const int e)
	{
		//client->request.status  = 400;
		body = "<html><body><h1>ERROR FILE NOT FOUND</h1></body></html>";
	}
	client->response.response = getResponseHeader(client->request, body) + body;
}
