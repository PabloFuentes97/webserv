#include "webserv2.hpp"

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

std::string getRequestedFile(HttpRequest *currentRequest) {

	std::string path = "/Users/anegrin-/Desktop/wgit/documents";
	currentRequest->status = 200;

	std::string filePath = path + currentRequest->url; 
		std::cout << "URL FOR REQUEST IS: " << filePath << std::endl;
	if (access(filePath.c_str(), F_OK) != 0) {
		filePath = path + "/error404.html"; 
		currentRequest->status = 404;
	}
	else if (access(filePath.c_str(), R_OK) != 0) {
		filePath = path + "/error403.html"; 
		currentRequest->status = 403;
	}
	if ((currentRequest->url).find('.') != std::string::npos) {
		std::string checkRequest = (currentRequest->url).substr((currentRequest->url).find('.'));
		std::cout << "CHECK REQUEST IS: " << checkRequest <<  std::endl;
		if (checkRequest == ".jpeg")
		filePath = "";
	}
	//si es un script (terminación) habrá q redirigir a CGI (ejecutar en un hijo);
	
	struct stat info;
    stat(filePath.c_str(), &info);
	if (S_ISDIR(info.st_mode) != 0)
		filePath = path + "/directory.html"; 

	std::cout << std::endl << "FILEPATH IS:" << filePath << std::endl; 
	return filePath; 
}

std::string getResponseBody(std::string fileToReturn) {

	std::ifstream file (fileToReturn);
	std::string fileLine;
	
    if (!file.is_open()) {
        std::cerr << "File error" << std::endl;
        exit (1); }

	char c;
	while (file.get(c))
		fileLine.push_back(c);
	file.close();
	std::cout << std::endl << "RESPONSE BODY IS: " << fileLine << std::endl;

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
	line.append("Content-Length: ");
	line.append(std::to_string((body).size()));
	line.append("\r\n");
	line.append("Connection: close");
	line.append("\r\n\r\n");

	return line;
}

std::string GetResponse(HttpRequest *request) {
	
	std::string fileToReturn = getRequestedFile(request);

	HttpResponse Response;
	//
	if (fileToReturn.empty())
		Response.body = "";
	else
		Response.body = getResponseBody(fileToReturn);
	Response.firstLine = getResponseFirstLine(*request, Response.body);
	std::string finalRequest = Response.firstLine + Response.body;
	
	return finalRequest;
}

std::string ResponseToMethod(HttpRequest *request) {
	
	std::string response = "";
	if (request->method == "GET")
		response = GetResponse(request);
	// else if (request.method == "POST")
	// 	response = PostResponse(request);
	return response;
}