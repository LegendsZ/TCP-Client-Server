#pragma once
#include <WS2tcpip.h>
#include <string>
#include <thread>
#include <vector>

#pragma comment (lib, "ws2_32.lib")

class client {
private:
	std::string ip_addy;
	int port;

	sockaddr_in hint;
	SOCKET sock;
	char buff[4096];
	
	std::thread listenThread;
	bool listen = false;
public:
	std::vector<std::string> recieved;

	client(std::string ip_addyG, int portG): ip_addy(ip_addyG), port(portG) {
		ZeroMemory(buff, 4096);
	}

	bool initWinsock() {
		WSADATA data;
		WORD ver = MAKEWORD(2, 2);
		int wsResult = WSAStartup(ver, &data);
		if (wsResult) {
			//std::cerr << "Winsock startup failure. Error #" << wsResult << "\n";
			return false;
		}
		return true;
	}
	bool createSocket() {
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET) {
			//std::cerr << "Socket creation failure. Error #" << WSAGetLastError() << "\n";
			WSACleanup();
			return false;
		}
		return true;
	}
	bool configHint() {
		hint.sin_family = AF_INET;
		hint.sin_port = htons(port);
		inet_pton(AF_INET, ip_addy.c_str(), &hint.sin_addr);
		return true;
	}
	bool connectServer() {
		int connRes = connect(sock, (sockaddr*)&hint, sizeof(hint));
		if (connRes == SOCKET_ERROR) {
			//std::cerr << "Connection failure. Error #" << WSAGetLastError() << "\n";
			closesocket(sock);
			WSACleanup();
			return false;
		}
		return true;
	}
	bool sendData(std::string userInputG) {
		int sendRes = send(sock, userInputG.c_str(), userInputG.size() + 1, 0);
		return true;
	}
	bool startListen() {
		listen = true;
		listenThread = std::thread(&client::listenLoop, this);
		return true;
	}
	bool stopListen() {
		listen = false;
		listenThread.detach();
		return true;
	}
	void listenLoop() {
		while (listen) {
			ZeroMemory(buff, 4096);
			int bytesRec = recv(sock, buff, 4096, 0);
			if (bytesRec > 0) {
				//echo response to console
				//std::cout << "\b\bServer: " << std::string(buff, 0, bytesRec) << "\n> ";
				recieved.push_back(std::string(buff, 0, bytesRec));
			}
		}
	}

	bool Terminate() {
		closesocket(sock);
		WSACleanup();
		return true;
	}

	bool autoStart() {
		initWinsock();
		createSocket();
		configHint();
		connectServer();
		startListen();
		return true;
	}

	bool setIP(std::string ip_addyG) {
		ip_addy = ip_addyG;
		return true;
	}
	std::string getIP() {
		return ip_addy;
	}

	bool setPORT(int portG) {
		port = portG;
		return true;
	}
	int getPORT() {
		return port;
	}
};