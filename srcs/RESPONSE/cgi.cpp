#include "../../includes/webserv.hpp"

char **getCgiEnv(std::string path, client* client) {

    std::vector<std::string> env;
    char **arrayEnv;
    
    std::string scriptName;;
    scriptName = "SCRIPT_NAME=" + path.substr(0, path.find('.') + 3);
    /*
    el final en .p por algún motivo da error 400 y no 404 xro no es mío eso
    crear el script del json a ser posible y si no un par de ejemplos generales
    */
    env.push_back(scriptName);

    std::string requestedMethod = "REQUEST_METHOD=" + client->request.method;
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
    
    std::string pathInfo = "PATH_INFO=" + client->request.pathInfo;
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
    //  std::cout << "Array env var is: " << arrayEnv[i] << std::endl;
    // }
    return (arrayEnv);

}

void CGIForward(client *client)
{
    std::string                 path;
    std::vector<std::string>    redirs;

    redirs.push_back("cgi_pass");
    path = getPathFileRequest(client, redirs);
	
    if (access(path.c_str(), F_OK) != 0)
        throw (NOT_FOUND);
    if (access(path.c_str(), X_OK) != 0)
        throw (INTERNAL_SERVER_ERROR);

    int pipes[2];
    int status;
    if (pipe(pipes) == -1)
       throw (INTERNAL_SERVER_ERROR);

    pid_t exec_pid = fork();
    if (exec_pid == -1)
        throw (INTERNAL_SERVER_ERROR);
    
    if (exec_pid == 0)
    {
        int fd[2];
        if (pipe(fd) == -1)
			throw(INTERNAL_SERVER_ERROR);
        std::cout << "path is: " << path << std::endl;
        char **cgiEnv = getCgiEnv(path, client);

		if (close(pipes[0]) == -1)
            throw (INTERNAL_SERVER_ERROR);
        if (dup2(pipes[1], STDOUT_FILENO) == -1)
            throw (INTERNAL_SERVER_ERROR);
        if (close(pipes[1]) == -1)
            throw (INTERNAL_SERVER_ERROR);
		write(fd[1], client->request.buf.c_str(), atoi((*getMultiMapValue(client->request.headers, "Content-Length")).c_str()));
		if (dup2(fd[0], STDIN_FILENO) == -1)
            throw (INTERNAL_SERVER_ERROR);
		if (close(fd[1]) == -1 || close(fd[0]) == -1)
            throw (INTERNAL_SERVER_ERROR);
        if (execve(path.c_str(), NULL, cgiEnv) != 0)
            throw(INTERNAL_SERVER_ERROR);
		throw(INTERNAL_SERVER_ERROR);
    }
    else if (exec_pid > 0) {

        time_t ref = std::time(NULL);
        time_t now;
        while (waitpid(exec_pid, &status, WNOHANG) == 0)
        {
            now = std::time(NULL);
            if (now > ref + CGITIMEOUT)
            {
                kill(exec_pid, SIGKILL);
                throw (GATEWAY_TIMEOUT); 
            }
        }
    }
    if (WEXITSTATUS(status) != 0)
        throw(BAD_GATEWAY);
    std::cout << "Estoy en proceso padre" << std::endl;
    if (close(pipes[1]) == -1)
        throw (INTERNAL_SERVER_ERROR);
    char    *readCGI = readFileSeLst(pipes[0]);
    if (close(pipes[0]) == -1)
        throw (INTERNAL_SERVER_ERROR);
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