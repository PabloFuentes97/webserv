#include "../../includes/webserv.hpp"

std::string	getStatus(int status)
{
	std::cout << "REQUEST STATUS: " << status << std::endl;
	switch (status) {
        case CREATED:
            return "201 Created";
		case MOVED_PERMANENTLY:
			return "301 Moved Permanently";
        case BAD_REQUEST:
            return "400 Bad Request"; //El formato de la request está mal
        case FORBIDDEN:
            return "403 Forbidden"; //Acceso no permitido
        case NOT_FOUND:
            return "404 Not Found"; //Archivo no encontrado
        case METHOD_NOT_ALLOWED:
            return "405 Method Not Allowed"; //Método que no gestionamos (DELETE de CGI o cualquier otro)
        case REQUEST_TIMEOUT:
            return "408 Request Timeout"; //Timeout
		case CONFLICT:
            return "409 Conflict";
		case LENGTH_REQUIRED:
            return "411 Length Required";
        case PAYLOAD_TOO_LARGE:
            return "413 Payload Too Large"; //Request muy grande
        case TOO_MANY_REQUESTS:
            return "429 Too Many Request"; //Muchas
        case INTERNAL_SERVER_ERROR:
            return "500 Internal Server Error";
        case NOT_IMPLEMENTED:
            return "501 Not Implemented";
        case BAD_GATEWAY:
            return "502 Bad Gateway";
        case GATEWAY_TIMEOUT:
            return "504 Gateway Timeout";
        default:
            return "200 OK";
    }
}

std::string	getErrorPath(struct client *client, int error)
{
	std::cout << "---------Hubo error: " << error << " , mandar error path--------" << std::endl;
	//mirar si en config hay ficheros de error redirigidos
	if (client->loc)
	{
		std::cout << "Buscar en location error_files" << std::endl;
		typedef std::multimap<std::string, std::string>::iterator itm;
		
		itm it = client->loc->context._dirs.find("error_files");
		if (it != client->loc->context._dirs.end())
		{
			std::pair<itm, itm> itr = client->loc->context._dirs.equal_range("error_files");
			std::cout << "Encuentra error_files en location" << std::endl;
			for (itm ib = itr.first, ie = itr.second; ib != ie; ib++)
			{
				std::cout << "Key: " << ib->first << " | Value: " << ib->second << std::endl;
				int	n = atoi(ib->second.c_str());
				std::cout << "error de error_files: " << n << std::endl;
				if (n == error)
				{
					ie--;
					std::cout << "Fichero a redirigir: " << "Key: " << ie->first << " | Value: " << ie->second << std::endl;
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
		//client->request.status  = BAD_REQUEST;
		body = "<html><body><h1>ERROR FILE NOT FOUND</h1></body></html>";
	}
	client->response.response = getResponseHeader(client->request, body) + body;
}
