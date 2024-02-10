#include "../../includes/webserv.hpp"

size_t	countCharinStr(const char *str, char c)
{
	int	count = 0;
	
	for (size_t i = 0; i < strlen(str); i++)
		if	(str[i] == c)
			count++;
	return (count);
}

int	cmpLocations(parseTree *loc, parseTree *cmp)
{
	std::string dir = loc->context._args[0];
	std::string	cmp_cast = cmp->context._args[0];
	
	int	slash1 = countCharinStr((dir).c_str(), '/');
	int	slash2 = countCharinStr((cmp_cast).c_str(), '/');
	if (slash1 > slash2)
		return (slash1 - slash2);
	if (slash1 < slash2)
		return (slash1 - slash2);
	if (dir == "/")
		return (1);

	std::string	s1;
	std::string	s2;
	for (size_t start1 = 1, start2 = 1, i = 1, j = 1; start1 < dir.length() && start2 < cmp_cast.length(); start1++, start2++)
	{
		for ( ; dir[start1] && dir[start1] != '/'; start1++);
		for ( ; cmp_cast[start2] && cmp_cast[start2] != '/'; start2++);
		s1 = dir.substr(i, start1 - i);
		s2 = cmp_cast.substr(j, start2 - j);
		if (s1 != s2)
			return (0);
		i = start1 + 1;
		j = start2 + 1;
	}
	return (1);
}

void	linearInsertLoc(std::vector<parseTree *> locs, parseTree *add)
{
	typedef std::vector<parseTree *>::iterator	itPt;
	itPt b = locs.begin();

	for (itPt e = locs.end(); b != e; b++)
	{
		if (cmpLocations(*b, add) > 0)
			break ;
	}
	locs.insert(b, add);
}