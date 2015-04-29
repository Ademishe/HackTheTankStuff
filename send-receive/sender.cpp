#include <iostream>
#include <boost/filesystem/fstream.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

namespace ba = boost::asio;

ba::io_service service;
ba::ip::tcp::endpoint ep(ba::ip::address::from_string("127.0.0.1"), 8001);
ba::ip::tcp::endpoint ep_for_key(ba::ip::address::from_string("127.0.0.1"), 8002);

std::string pass("StDSGOpvfq3/24");

char mr_sender_online[] = "sender_online";
char mr_receiver_online[] = "receiver_online";

size_t read_complete(char *buf, const boost::system::error_code& err, size_t bytes) {
	if (err) return 0;
	bool found = std::find(buf, buf + bytes, '\n') < buf + bytes;
	return found ? 0 : 1;
}

bool waitForReceiver() {
	ba::ip::tcp::acceptor acceptor(service, ba::ip::tcp::endpoint(ba::ip::tcp::v4(), 8001));
	ba::ip::tcp::socket sock(service);
	try {
		char buff[1024];
		acceptor.accept(sock);
		int bytes = ba::read(sock, ba::buffer(buff), boost::bind(read_complete, buff, _1, _2));

		std::string response(buff, bytes - 1);
		if (response.find(mr_receiver_online) == std::string::npos) {
			sock.close();
			return false;
		}
		else {
			sock.write_some(ba::buffer(std::string(mr_sender_online) + "\n"));
			sock.close();
			return true;
		}
	}
	catch (...) {
		sock.close();
		return false;
	}
}

bool notifyReceiver() {
	ba::ip::tcp::socket sock(service);

	try {
		char buff[1024];
		sock.connect(ep);
		sock.write_some(ba::buffer(std::string(mr_sender_online) + "\n"));
		int bytes = ba::read(sock, ba::buffer(buff), boost::bind(read_complete, buff, _1, _2));
		sock.close();

		std::string response(buff, bytes - 1);
		if (response.find(mr_receiver_online) == std::string::npos) return false;
		else return true;
	}
	catch (...) {
		sock.close();
		return false;
	}
}

void sendKey() {
	ba::ip::tcp::socket sock(service);
	try {
		std::string c = pass;
		bool flag = true;
		for (int i = 0; i < c.length(); ++i) {
			if (flag) {
				c[i] = char(c[i] - 1);
			}
			else {
				c[i] = char(c[i] + 1);
			}
			flag = !flag;
		}

		sock.connect(ep_for_key);
//        std::cout << pass;
		sock.write_some(ba::buffer(c + "\n"));
		sock.close();
	}
	catch (...) {
		std::cout << "Some error occured while transfering." << std::endl;
		sock.close();
	}
}

void sendFile() {
	ba::ip::tcp::socket sock(service);
	boost::filesystem::ifstream inFile("data", std::ios::binary | std::ios::in);
	char filebuf[20000];
	inFile.read(filebuf, sizeof(filebuf));
	if (!inFile.is_open()) {
		sock.connect(ep);
		sock.close();
		std::cout << "No data to send." << std::endl;
		return;
	}

	try {
		sock.connect(ep);
		//std::cout << inFile.gcount() << std::endl;
		ba::write(sock, ba::buffer(filebuf, inFile.gcount()));
		sock.close();
	}
	catch (...) {
		std::cout << "Some error occured while transfering." << std::endl;
		sock.close();
	}
}

void senderTransferData() {
	std::cout << "Everything is OK. Beginning to transfer the data." << std::endl;

	boost::thread_group threads;
	threads.create_thread(sendFile);
	threads.create_thread(sendKey);
	threads.join_all();
	std::cout << "The transfering has been finished." << std::endl;
}

void senderRoutine() {
	std::cout << "Checking Mr. Receiver to be online." << std::endl;
	if (!notifyReceiver()) {
		std::cout << "Mr. Receiver is offline. Waiting for him." << std::endl;
		if (!waitForReceiver()) {
			std::cout << "Unknown response. Quitting." << std::endl;
			return;
		}
		else std::cout << "Mr. Receiver has got online." << std::endl;
	}
	senderTransferData();
}

int main(int argc, char* argv[]) {
	std::cout << "Hello! I'm Mr. Sender and I'm going to send some data to Mr. Receiver." << std::endl;
	boost::thread t(senderRoutine);
	t.join();
	return 0;
}
