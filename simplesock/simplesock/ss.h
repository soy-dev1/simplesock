#pragma once
#include <windows.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#pragma comment (lib, "Ws2_32.lib")

#define MESSAGE_SIZE sizeof(message)
#define DATA_SIZE 2048


struct message {
	int messageType;
	int numPackets;
	int seqNum;
	char data[DATA_SIZE];
};

struct connection {
	int id;
	SOCKET sock;
	std::thread* worker;
};

class SimpleServer {
private:
	sockaddr_in addr{};
	std::vector<connection> connections; 
public:
	void init(const char* IP, int ipType, int port, void (*clientHandlerFunction)(connection&, SimpleServer*)); // sets up listener 
	void close(int id);
	int sendData(char* inBuf, int size, int id);
	char* receive(SOCKET &sock);
};

class SimpleClient {
private:
	sockaddr_in addr{};
	SOCKET sock{};
public:
	void init(const char* IP, int ipType, int port);
	void close();
	int sendData(char* inBuf, int size);
	char* receive();
};