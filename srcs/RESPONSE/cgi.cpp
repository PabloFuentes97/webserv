#include "../../includes/webserv.hpp"

char **getCgiEnv(std::string &path, client* client) {

	std::vector<std::string> env;
	char **arrayEnv;
	
	std::string scriptName;
	if (path.find('?') != std::string::npos)
		scriptName = "SCRIPT_NAME=" + path.substr(0, path.find('?') + 1);
	else
		scriptName = "SCRIPT_NAME=" + path;
	env.push_back(scriptName);

	std::string requestedMethod = "REQUESTED_METHOD=" + client->request.method;
	env.push_back(requestedMethod);

	std::string contentLength = "CONTENT_LENGTH=0";
	size_t contentLengthVal = 0;

	if (client->request.method == "POST")
	{
		std::cout << "----CGI POST-----" << std::endl;
		std::string *contentLengthStr = getMultiMapValue(client->request.headers, "Content-Length");
		if (contentLengthStr)
		{
			contentLengthVal = atoi((*getMultiMapValue(client->request.headers, "Content-Length")).c_str());
			contentLength = "CONTENT_LENGTH=" + *contentLengthStr;
		}

		client->request.query = client->request.buf.substr(0, contentLengthVal);
		std::cout << "BUF IS: " << client->request.buf << std::endl;
		std::cout << "POST QUERY IS: " << client->request.query << std::endl;
	}
	std::string queryString = "QUERY_STRING=" + client->request.query;
	env.push_back(queryString);
		
	env.push_back(contentLength);
	
	std::string pathInfo = "PATH_INFO=" + path;
	env.push_back(pathInfo);
	
	for (std::vector<std::string>::iterator it = env.begin(); it != env.end(); ++it)
		std::cout << "Vector env var is: " << *it << std::endl;

	std::cout << "----------" << std::endl;

	arrayEnv = (char **)malloc(sizeof(char *) * (env.size() + 1));
	if (!arrayEnv)
		exit(1);
	size_t i = 0;
	for (std::vector<std::string>::iterator it = env.begin(); it != env.end(); it++)
	{
		std::cout << "Variable is: " << (*it).size() << std::endl;
		std::cout << "Variable size is: " << *it << std::endl << std::endl;
		arrayEnv[i] = strdup((char *)(*it).c_str());
		if (!arrayEnv[i])
			exit(1);
		std::cout << "Arrenv is: " << *it << std::endl << std::endl;
		i++;
	}
	arrayEnv[i] = NULL;

	// for (int i = 0; arrayEnv[i]; i++) {
	// 	std::cout << "Array env var is: " << arrayEnv[i] << std::endl;
	// }
	return (arrayEnv);

}

void CGIForward(client *client)
{
	std::string 				path;
	std::vector<std::string>	redirs;

	redirs.push_back("alias");
	redirs.push_back("root");
	path = getRequestedFile(client, redirs);
	std::cout << "filePath: " << path << std::endl;
	if (access(path.c_str(), F_OK) != 0)
		throw (404);
	if (access(path.c_str(), X_OK) != 0)
		throw (500);
    int pipes[2];
	int status;
    if (pipe(pipes) == -1)
        exit (1);

    pid_t exec_pid = fork();
	
	if (exec_pid > 0) {

		time_t ref = std::time(NULL);
		time_t now;
		while (waitpid(exec_pid, &status, WNOHANG) == 0)
		{
			now = std::time(NULL);
			if (now > ref + CGITIMEOUT)
			{
				kill(exec_pid, SIGKILL);
				client->request.status = 508;
				throw (508); 
			}
		}
	}
    else if (exec_pid == 0)
    {
        std::cout << "path is: " << path << std::endl;
    	char **cgiEnv = getCgiEnv(path, client);
		std::string newPath = path.substr(0, path.find('?'));
		for (int i = 0; cgiEnv[i]; i++) {
			std::cout << "Array env var is: " << cgiEnv[i] << std::endl;
		}
        dup2(pipes[1], STDOUT_FILENO);
        close(pipes[0]);
        if (execve(newPath.c_str(), NULL, cgiEnv) < 0) {
            throw(500);
        }
    }
    std::cout << "Estoy en proceso padre" << std::endl;
    close(pipes[1]);
    char    *readCGI = readFileSeLst(pipes[0]);
	close(pipes[0]);
	std::string CGIstring = readCGI;
	free(readCGI);
    std::cout << "Fichero leído: " << CGIstring << std::endl;

	client->request.status = 200;
	HttpResponse Response;
	Response.body = CGIstring;
	Response.firstLine = getResponseHeader(client->request, Response.body);
	client->response.response = Response.firstLine + Response.body;

    return ;
}