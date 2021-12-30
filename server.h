#pragma once

#include <WS2tcpip.h>
#include <string>
#include <thread>
#include <vector>

class server {
private:

	SOCKET listening = NULL;
	int port;
	fd_set master;
	char buff[4096];

	std::thread findingConnectionsThread;
	std::thread listenThread;
	bool isFindingConnections = false;
	bool isListening = false;

public:
	std::vector<std::string> recieved;

	server(int portG) : port(portG) {}

	bool autoStart() {
		initWinsock();
		createSocket();
		bindSocket();
		configSocket();
		startFindConnections();
		return true;
	}

	bool initWinsock() {
		WSADATA wsData;
		WORD ver = MAKEWORD(2, 2);
		int wsOk = WSAStartup(ver, &wsData);

		if (wsOk) {
			return false;
		}
		return true;
	}

	bool createSocket() {
		listening = socket(AF_INET, SOCK_STREAM, 0);
		if (listening == INVALID_SOCKET) {
			return false;
		}
		return true;
	}

	bool bindSocket() {
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(port);
		hint.sin_addr.S_un.S_addr = INADDR_ANY;
		bind(listening, (sockaddr*)&hint, sizeof(hint));
		return true;
	}

	bool configSocket() {
		listen(listening, SOMAXCONN);
		FD_ZERO(&master);
		FD_SET(listening, &master);
		return true;
	}

	bool startFindConnections() {
		isFindingConnections = true;
		findingConnectionsThread = std::thread(&server::findConnectionsLoop, this);
		return true;
	}

	bool endFindConnections() {
		isFindingConnections = false;
		findingConnectionsThread.detach();
		return true;
	}

	void findConnectionsLoop() {
		while (isFindingConnections) {
			fd_set copy = master;
			int socketCnt = select(0, &copy, nullptr, nullptr, nullptr);
			for (int i = 0; i < socketCnt; i++) {
				if (!isFindingConnections) {
					return;
				}
				SOCKET sock = copy.fd_array[i];
				if (sock == listening) {
					//accept new conn
					SOCKET client = accept(listening, nullptr, nullptr);

					//add new conn to list of connected sockets/clients
					FD_SET(client, &master);
					//reply back

					sockaddr_in clientAddr;
					char host[NI_MAXHOST];
					char service[NI_MAXHOST];
					ZeroMemory(host, NI_MAXHOST);
					ZeroMemory(service, NI_MAXHOST);
					if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXHOST, 0) == 0) {
						//std::cout << host << " connected on port " << service << "\n";
					}
					else {
						inet_ntop(AF_INET, &clientAddr.sin_addr, host, NI_MAXHOST);
						//std::cout << host << " connected on port " << ntohs(clientAddr.sin_port) << "\n";
					}

					std::string welcomeMSG = "You have joined.";
					send(client, welcomeMSG.c_str(), welcomeMSG.size() + 1, 0);
				}
			}
		}
	}

	bool startListening() {
		isListening = true;
		listenThread = std::thread(&server::listenLoop, this);
		return true;
	}

	bool endListening() {
		isListening = false;
		listenThread.detach();
		return true;
	}

	void listenLoop() {
		while (isListening) {
			for (int i = 0; i < master.fd_count; i++) {
				SOCKET clientSocket = master.fd_array[i];
				ZeroMemory(buff, 4096);
				int bytesRecieved = recv(clientSocket, buff, 4096, 0);
				if (bytesRecieved == SOCKET_ERROR || bytesRecieved == 0) {
					continue;
				}
				recieved.push_back(getMsg(buff, bytesRecieved));
				
			}
		}
	}

	bool sendData(int index, std::string msgG) {
		SOCKET clientSocket = master.fd_array[index];
		char buffs[6] = "hello";
		send(clientSocket, msgG.c_str(), msgG.size()+1, 0);
		return true;
	}

	bool Terminate() {
		WSACleanup();
		return true;
	}

	std::string getMsg(char* buffG, const int length) {
		std::string toRet = "";
		for (int i = 0; i < length; i++) {
			toRet += buffG[i];
		}
		return toRet;
	}
};