#include <string>
#include <iostream>

int	main()
{
	std::string	p("egege");

	std::cout << "Size: " << p.size() << " , capacity: " << p.capacity() << std::endl;
	p.reserve(p.size() + 100);
	std::cout << "Size: " << p.size() << " , capacity: " << p.capacity() << std::endl;
	std::cout << p << std::endl;
}