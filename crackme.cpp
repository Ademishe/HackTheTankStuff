#include <iostream>
#include <string>
#include <functional>
//#include <stdio.h>

std::string success_str("Sucess");
std::string fail_str("Fail");
static const int max_len = 30;

int main(int argc, char *argv[]) {
	std::string login, pass;
	std::cout << "Enter login:";
	std::getline(std::cin, login);
	login.erase(remove_if(login.begin(), login.end(), isspace), login.end());
	std::cout << login << std::endl;
	
	
	std::cout << "Enter password:";
	std::getline(std::cin, pass);
	std::cout << std::endl;
	
	std::hash <std::string> ptr_hash;
	std::string hash_str(std::to_string(ptr_hash(login)));
	char *buff = new char[hash_str.length()];;
	std::size_t length = hash_str.copy(buff, max_len);
	
	char temp = buff[0];
	for (int i = 0; i < hash_str.length(); ++i) {
		if (i == length - 1) {
			buff[i] ^= temp;
		}
		else {
			buff[i] ^= buff[i+1];
		}
		buff[i] += 0x30;
//		printf("0x%x --- 0x%x\n", hash_str[i], buff[i]);
	}
	std::string res(buff);
	delete[] buff;
	if (res.compare(pass) != 0) {
		std::cout << fail_str << std::endl;
//		std::cout << res << std::endl;
	}
	else {
		std::cout << success_str << std::endl;
	}
	return 0;
}