#include "ss.h"

int findLowestId() {

}

void SimpleServer::init(const char* IP, int ipType, int port, void (*clientHandlerFunction)(connection&, SimpleServer*)) {

	WSAData wsadata;
	SOCKET listener;
	static int idcount;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	addr.sin_family = ipType;
	addr.sin_addr.S_un.S_addr = inet_addr(IP);
	addr.sin_port = htons(port);

	listener = socket(ipType, SOCK_STREAM, IPPROTO_TCP);
	if (!bind(listener, (sockaddr*)&addr, sizeof(addr))) {
		listen(listener, SOMAXCONN);

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2);
		std::cout << "[+] Server Initialized! IP: " << IP << " Port: " << port << "\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);

		while (true) {
			connection newcl;
			newcl.sock = accept(listener, NULL, NULL);
			newcl.id = ++idcount;
			newcl.worker = new std::thread(clientHandlerFunction, std::ref(newcl), this);
			newcl.worker->detach();

			connections.push_back(newcl);
		}
	}

}

void SimpleServer::close(int id) {
	for (connection& client : connections) {
		if (client.id == id) {
			std::cout << "[*] Client disconnected\n";
			shutdown(client.sock, 2);
			delete client.worker;
		}
	}
}

int SimpleServer::sendData(char* inBuf, int size, int id) {
	std::vector<char*> bufvec;
	message out{};
	SOCKET sock{};
	int sent = 0;
	int numMsg = (size / DATA_SIZE) + 1;
	int remainder = size % DATA_SIZE;
	int curmsg = 0;

	for (connection& client : connections) {
		if (client.id == id)
			sock = client.sock;
	}

	if (numMsg > 1 && remainder == 0) {
		for (int i = 0; i < numMsg; ++i) {
			char* newBuf = new char[DATA_SIZE];
			memset(newBuf, 0, DATA_SIZE);
			memcpy(newBuf, inBuf + (i * DATA_SIZE), DATA_SIZE);
			bufvec.push_back(newBuf);
		}
	}
	else if (numMsg > 1) {
		for (int i = 0; i != numMsg - 1; ++i) {
			char* newBuf = new char[DATA_SIZE];
			memset(newBuf, 0, DATA_SIZE);
			memcpy(newBuf, inBuf + (i * DATA_SIZE), DATA_SIZE);
			bufvec.push_back(newBuf);
		}
		char* last = new char[DATA_SIZE];
		memset(last, 0, DATA_SIZE);
		memcpy(last, inBuf + ((numMsg - 1) * DATA_SIZE), remainder);
		bufvec.push_back(last);
	}
	else {
		char* data = new char[DATA_SIZE];
		memset(data, 0, DATA_SIZE);
		memcpy(data, inBuf, size);
		bufvec.push_back(data);
	}

	for (char* buf : bufvec) {
		out.messageType = 0;
		out.numPackets = numMsg;
		out.seqNum = ++curmsg;
		memcpy(out.data, buf, DATA_SIZE);
		sent += send(sock, reinterpret_cast<char*>(&out), MESSAGE_SIZE, 0);
		std::cout << buf;
		delete[] buf;
	}

	return sent;
}

char* SimpleServer::receive(SOCKET &sock) {

	message msg{};
	recv(sock, reinterpret_cast<char*>(&msg), MESSAGE_SIZE, 0);

	if (msg.messageType == 1) {
		return nullptr;
	}

	if (msg.seqNum != msg.numPackets) {
		char* large = new char[msg.numPackets * DATA_SIZE];
		while (msg.seqNum != msg.numPackets) {
			memcpy(large + ((msg.seqNum - 1) * DATA_SIZE), msg.data, DATA_SIZE);
			recv(sock, reinterpret_cast<char*>(&msg), MESSAGE_SIZE, 0);
		}
		memcpy(large + ((msg.seqNum - 1) * DATA_SIZE), msg.data, DATA_SIZE);
		recv(sock, reinterpret_cast<char*>(&msg), MESSAGE_SIZE, 0);
		return large;
	}
	
	char* data = new char[DATA_SIZE];
	memcpy(data, msg.data, DATA_SIZE);
	return data;
}