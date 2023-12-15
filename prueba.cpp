#include <algorithm>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include "webserv.hpp"
#include <string>

int	delete_method(std::string &loc)
{
	struct stat	st;

	if (stat(loc.c_str(), &st) == 0 && !(st.st_mode & S_IFDIR))
        std::cout << "The path is valid!";
    else
        std::cout << "The Path is invalid!";
}


int	main(void)
{
	std::string str = "/Users/marias-e/Desktop/webserv/a.out";
	delete_method(str);
}