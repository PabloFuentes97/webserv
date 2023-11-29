#include "webserv.hpp"

int	main(int argc, char **argv)
{
	//lado del servidor
	//crear socket
	if (argc != 2)
	{
		std::cout << "Invalid number of args!" << std::endl;
		return (1);
	}
	std::cout << "Fichero a abrir: " << argv[1] << std::endl;
	int	fileFd = open(argv[1], O_RDONLY);
	if (fileFd < 0)
	{
		perror("open failed: ");
		return (1);
	}
	parseFile(fileFd);
	int socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFd < 0)
	{
		perror("socket failed: ");
		return (1);
	}
	//estructura sockaddr_in para pasarsela a bind y accept
	struct sockaddr_in	sock;
	bzero(&sock, sizeof(sock));
	sock.sin_family = AF_INET;
	sock.sin_port = htons(8080);
	sock.sin_addr.s_addr = htonl(INADDR_ANY); // o inet_addr("0.0.0.0")
	std::cout << "Puerto con orden de bytes de red: " << sock.sin_port << std::endl;
	std::cout << "Dirección con orden de bytes de red: " << sock.sin_addr.s_addr << std::endl;
	//setear el socket a modo "non-block"
	int status = fcntl(socketFd, F_SETFL, O_NONBLOCK); //setsockopt(socketFd, SOL_SOCKET, )
	int yes = 1;
	setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes , sizeof(yes)); //para que bind no se quede parado
	//asigna nombre único al socket
	if (bind(socketFd, (sockaddr*)&sock, sizeof(sock)) < 0)
	{
		perror("Bind failed: ");
		return (2);
	}		
	//int status = fcntl(socketFd, F_SETFL, fcntl(socketFd, F_GETFL, 0) | O_NONBLOCK);
	//conectarse al servidor usando direccion ip
	//if (connect(socketFd, (sockaddr*)&sock, sizeof(sock)) < 0)
	//	return (4);
	//socket escucha conexiones entrantes y las guarda en una queue con capacidad limitada
	if (listen(socketFd, 1000) < 0)
	{
		perror("Listen failed: ");
		return (3);
	}
	//connectionSelect(socketFd);
	//connectionPoll(socketFd);
	//connectionKQueue(socketFd);
	close(socketFd);
	//unlink(socketName);
	return (0);
}
