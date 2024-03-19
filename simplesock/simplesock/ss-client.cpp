#include "ss.h"

void SimpleClient::init(const char* IP, int ipType, int port) {

	WSAData wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	addr.sin_family = ipType;
	addr.sin_addr.S_un.S_addr = inet_addr(IP);
	addr.sin_port = htons(port);

	sock = socket(ipType, SOCK_STREAM, IPPROTO_TCP);
	if (!connect(sock, (sockaddr*)&addr, sizeof(addr))) {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2);
		std::cout << "[+] Connected to server! IP: " << IP << " Port: " << port << "\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
	}
	return;
}

void SimpleClient::close() {
	message out{};
	out.messageType = 1;
	send(sock, reinterpret_cast<char*>(&out), MESSAGE_SIZE, 0);
	shutdown(sock, 2);
}

int SimpleClient::sendData(char* inBuf, int size) {
	std::vector<char*> bufvec;
	message out{};
	int sent = 0;
	int numMsg = (size / DATA_SIZE) + 1;
	int remainder = size % DATA_SIZE;
	int curmsg = 0;

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

char* SimpleClient::receive() {
	message msg{};
	recv(sock, reinterpret_cast<char*>(&msg), MESSAGE_SIZE, 0);

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