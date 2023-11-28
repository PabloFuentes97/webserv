/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pfuentes <pfuentes@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/09 13:03:09 by pfuentes          #+#    #+#             */
/*   Updated: 2023/11/27 11:42:43 by pfuentes         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.hpp"

static char	*aux_join(char *s1, char *s2)
{
	char *aux = s1;
	char	*join = ft_strjoin(s1, s2);
	free(aux);
	return (join);
}

static int	ft_str_newline(char *str)
{
	int	cont;

	cont = 0;
	if (!str)
		return (0);
	while (str[cont])
	{
		if (str[cont] == '\n')
			return (1);
		cont++;
	}
	return (0);
}

static char	*create_new_stash(char *stash)
{
	char	*new_stash;
	int		cont;

	if (!stash)
		return (NULL);
	cont = 0;
	new_stash = NULL;
	while (stash[cont] && stash[cont] != '\n')
		cont++;
	if (stash[cont + 1] == '\0' || stash[cont] == '\0')
		free (stash);
	else
	{
		new_stash = ft_substr(stash, cont + 1, (ft_strlen(stash)) - cont + 1);
		free (stash);
		if (!new_stash)
			return (NULL);
	}
	return (new_stash);
}

static char	*create_line(char *stash)
{
	char	*line;
	size_t	cont;

	cont = 0;
	if (!stash)
		return (NULL);
	while (stash[cont] && stash[cont] != '\n')
		cont++;
	if (stash[cont] == '\n')
		line = ft_substr(stash, 0, cont + 1);
	else
		line = ft_strdup(stash);
	if (!line)
		return (NULL);
	return (line);
}

static char	*create_stash(char *stash, int fd)
{
	char	*buf;
	int		bytes;

	buf = (char *)calloc(BUFFER_SIZE + 1, sizeof(char));
	if (!buf)
		return (NULL);
	bytes = 1;
	while (bytes > 0 && ft_str_newline(stash) == 0)
	{
		bytes = read(fd, buf, BUFFER_SIZE);
		if (bytes == -1 || buf == NULL)
			return (NULL);
		buf[bytes] = '\0';
		if (bytes > 0)
		{
			if (!stash)
				stash = (char *)calloc(1, 1);
			stash = aux_join(stash, buf);
		}
	}
	if (buf)
		free (buf);
	return (stash);
}

char	*get_next_line(int fd)
{
	char static	*stash;
	char		*line;

	if (fd < 0 || BUFFER_SIZE <= 0 || read(fd, 0, 0) < 0)
		return (NULL);
	stash = create_stash(stash, fd);
	if (!stash)
		return (NULL);
	line = create_line(stash);
	if (!line)
		return (NULL);
	stash = create_new_stash(stash);
	return (line);
}
