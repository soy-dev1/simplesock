#include "simplesock/ss.h"

void serverThread(connection& client, SimpleServer* server);

int main() {
	SimpleServer server;
	server.init("127.0.0.1", AF_INET, 5050, &serverThread);
}

void serverThread(connection& client, SimpleServer* server) {
	std::cout << "[*] Client connected\n";

	while (true) {
		char* crap = server->receive(client.sock);

		if (crap == nullptr) {
			server->close(client.id);
			return;
		}

		std::cout << crap;
		delete[] crap;
	}
}