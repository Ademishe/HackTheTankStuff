#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/iostreams/stream.hpp>

namespace ba = boost::asio;

ba::io_service service;
ba::ip::tcp::endpoint ep(ba::ip::address::from_string("127.0.0.1"), 8001);

char mr_sender_online[] = "sender_online";
char mr_receiver_online[] = "receiver_online";

size_t read_complete_eof(char *buff, const boost::system::error_code& err, size_t bytes) {
	if (err) return 0;
	bool found = std::find(buff, buff + bytes, EOF) < buff + bytes;
	return found ? 0 : 1;
}

size_t read_complete(char *buff, const boost::system::error_code& err, size_t bytes) {
	if (err) return 0;
	bool found = std::find(buff, buff + bytes, '\n') < buff + bytes;
	return found ? 0 : 1;
}

bool waitForSender() {
	ba::ip::tcp::acceptor acceptor(service, ba::ip::tcp::endpoint(ba::ip::tcp::v4(), 8001));
	ba::ip::tcp::socket sock(service);
	try {
		char buff[1024];
		acceptor.accept(sock);
		int bytes = ba::read(sock, ba::buffer(buff), boost::bind(read_complete, buff, _1, _2));

		std::string response(buff, bytes - 1);
		if (response.find(mr_sender_online) == std::string::npos) {
			sock.close();
			return false;
		}
		else {
			sock.write_some(ba::buffer(std::string(mr_receiver_online) + "\n"));
			sock.close();
			return true;
		}
	}
	catch (...) {
		sock.close();
		return false;
	}
}

bool notifySender() {
	ba::ip::tcp::socket sock(service);

	try {
		char buff[1024];
		sock.connect(ep);
		sock.write_some(ba::buffer(std::string(mr_receiver_online) + "\n"));
		int bytes = ba::read(sock, ba::buffer(buff), boost::bind(read_complete, buff, _1, _2));
		sock.close();

		std::string response(buff, bytes - 1);
		if (response.find(mr_sender_online) == std::string::npos) return false;
		else return true;
	}
	catch (...) {
		sock.close();
		return false;
	}
}

void receiveKey() {
	ba::ip::tcp::acceptor acceptor(service, ba::ip::tcp::endpoint(ba::ip::tcp::v4(), 8002));
	ba::ip::tcp::socket sock(service);
	try {
		char buff[1024];
		acceptor.accept(sock);
		int bytes = ba::read(sock, ba::buffer(buff), boost::bind(read_complete, buff, _1, _2));

		std::string response(buff, bytes - 1); //this is password for archive.
//		std::cout << response << std::endl;
		sock.close();
	}
	catch (...) {
		std::cout << "Some error occured while transfering." << std::endl;
		sock.close();
	}
}

void receiveFile() {
	ba::ip::tcp::acceptor acceptor(service, ba::ip::tcp::endpoint(ba::ip::tcp::v4(), 8001));
	ba::ip::tcp::socket sock(service);
	boost::asio::streambuf buff;
	try {
		acceptor.accept(sock);
		int len = ba::read(sock, buff, boost::asio::transfer_at_least(17496));
		std::cout << len << std::endl;
		sock.close();
	}
	catch (std::exception &e) {
		std::cout << "Some error occured while transfering: " << e.what() << std::endl;
		sock.close();
	}
}

void receiverTransferData() {
	std::cout << "Everything is OK. Beginning to transfer the data." << std::endl;

	boost::thread_group threads;
	threads.create_thread(receiveFile);
	threads.create_thread(receiveKey);
	threads.join_all();
	std::cout << "The transfering has been finished." << std::endl;
}

void receiverRoutine() {
	std::cout << "Checking Mr. Sender to be online." << std::endl;
	if (!notifySender()) {
		std::cout << "Mr. Sender is offline. Waiting for him." << std::endl;
		if (!waitForSender()) {
			std::cout << "Unknown response. Quitting." << std::endl;
			return;
		}
		else std::cout << "Mr. Sender has got online." << std::endl;
	}
	receiverTransferData();
}



int main(int argc, char* argv[]) {
	std::cout << "Hello! I'm Mr. Receiver and I'm going to receive some data from Mr. Sender." << std::endl;
	boost::thread t(receiverRoutine);
	t.join();
	return 0;
}
