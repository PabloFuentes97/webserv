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
