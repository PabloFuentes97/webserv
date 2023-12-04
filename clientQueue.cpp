#include "webserv.hpp"

int	clientQueue::getPos(int checkFd) {
	for (int i = 0; i < clientArray.size(); i++)
		if (clientArray[i].fd == checkFd)
			return i;
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

void clientQueue::addClient(int clientFd, int servID) {
	if (getPos(clientFd) == -1 && clientFd <= SOMAXCONN)
	{
		struct client newClient;
		newClient.fd = clientFd;
		newClient.serverID = servID;
		newClient.request.method = "";
		newClient.request.url = "";
		newClient.request.body = "";
		newClient.request.status = 0;
		clientArray.push_back(newClient);
	}
}
