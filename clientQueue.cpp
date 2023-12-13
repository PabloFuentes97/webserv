#include "webserv.hpp"

int	clientQueue::getPos(int checkFd) {
	for (int i = 0; i < clientArray.size(); i++)
		if (clientArray[i].fd == checkFd)
			return i;
	return -1;
}

int	clientQueue::getServerId(int checkFd) {
	for (int i = 0; i < clientArray.size(); i++)
		if (clientArray[i].fd == checkFd)
		{
			std::cout << "Fd de socket: " << checkFd << ", serverID: " << clientArray[i].serverID << std::endl;
			return clientArray[i].serverID;
		}
			
	return -1;
}

clientQueue::clientQueue() { }

clientQueue::~clientQueue() { }

void clientQueue::clearRequest(int clientFd) {

	clientArray[getPos(clientFd)].request.method = "";
	clientArray[getPos(clientFd)].request.url = "";
	clientArray[getPos(clientFd)].request.body = "";
	clientArray[getPos(clientFd)].request.status = 0;
	//server = NULL;
}

void clientQueue::addClient(int clientFd, int serverFd) {

	struct client newClient;
	int	pos = getPos(clientFd);
	if (pos == -1 && clientFd <= SOMAXCONN)
	{
		struct client newClient;
		newClient.fd = clientFd;
		newClient.serverID = serverFd - 3;
		newClient.request.method = "";
		newClient.request.url = "";
		newClient.request.body = "";
		newClient.request.status = 0;
		clientArray.push_back(newClient);
	}
	else
	{
		this->clientArray[pos].fd = clientFd;
		this->clientArray[pos].serverID = serverFd - 3;
		this->clientArray[pos].request.method = "";
		this->clientArray[pos].request.url = "";
		this->clientArray[pos].request.body = "";
		this->clientArray[pos].request.status = 0;
	}
}
