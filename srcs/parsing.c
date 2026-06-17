#include "../srcs/lem_in.h"

/* Utilitaire pour lire une ligne depuis l'entrée standard */
char	*get_next_line(int fd)
{
	char	buffer[1024];
	int		ret;

	ret = read(fd, buffer, 1023);
	if (ret <= 0)
		return (NULL);
	buffer[ret] = '\0';
	return (strdup(buffer));
}

/* Split une string par un caractère délimiteur */
static char	**ft_split(char const *s, char c)
{
	char	**result;
	int		count;
	int		i;
	int		j;
	int		start;

	count = 0;
	i = 0;
	while (s && s[i])
	{
		if (s[i] != c && (i == 0 || s[i - 1] == c))
			count++;
		i++;
	}
	result = malloc(sizeof(char *) * (count + 1));
	if (!result)
		return (NULL);
	i = 0;
	j = 0;
	while (j < count)
	{
		while (s[i] == c)
			i++;
		start = i;
		while (s[i] && s[i] != c)
			i++;
		result[j] = malloc(sizeof(char) * (i - start + 1));
		if (!result[j])
			return (NULL);
		strncpy(result[j], s + start, i - start);
		result[j][i - start] = '\0';
		j++;
	}
	result[j] = NULL;
	return (result);
}

/* Libère les ressources allouées */
void	free_lemin(t_lemin *lemin)
{
	int	i;

	if (!lemin)
		return ;
	if (lemin->rooms)
	{
		i = 0;
		while (i < lemin->nb_rooms)
		{
			free(lemin->rooms[i].name);
			i++;
		}
		free(lemin->rooms);
	}
	if (lemin->links)
		free(lemin->links);
	if (lemin->adjacency)
	{
		i = 0;
		while (i < lemin->nb_rooms)
		{
			free(lemin->adjacency[i]);
			i++;
		}
		free(lemin->adjacency);
	}
}

/* Trouve la room par son nom */
int	find_room_by_name(t_lemin *lemin, const char *name)
{
	int	i;

	i = 0;
	while (i < lemin->nb_rooms)
	{
		if (strcmp(lemin->rooms[i].name, name) == 0)
			return (i);
		i++;
	}
	return (-1);
}

/* Parse une room */
int	parse_room(t_lemin *lemin, char *line, t_room_type type)
{
	char	**split;
	int		x;
	int		y;
	int		id;

	split = ft_split(line, ' ');
	if (!split || !split[0] || !split[1] || !split[2] || split[3])
	{
		ft_putendl_fd("ERROR", 2);
		return (-1);
	}
	if (split[0][0] == 'L' || split[0][0] == '#')
	{
		ft_putendl_fd("ERROR", 2);
		return (-1);
	}
	x = atoi(split[1]);
	y = atoi(split[2]);
	
	lemin->rooms = realloc(lemin->rooms, sizeof(t_room) * (lemin->nb_rooms + 1));
	if (!lemin->rooms)
		return (-1);
	
	id = lemin->nb_rooms;
	lemin->rooms[id].name = strdup(split[0]);
	lemin->rooms[id].x = x;
	lemin->rooms[id].y = y;
	lemin->rooms[id].type = type;
	lemin->rooms[id].id = id;
	
	if (type == START)
		lemin->start_id = id;
	if (type == END)
		lemin->end_id = id;
	
	lemin->nb_rooms++;
	return (0);
}

/* Parse un lien entre deux rooms */
int	parse_link(t_lemin *lemin, char *line)
{
	char	**split;
	int		room1_id;
	int		room2_id;

	split = ft_split(line, '-');
	if (!split || !split[0] || !split[1] || split[2])
	{
		ft_putendl_fd("ERROR", 2);
		return (-1);
	}
	
	room1_id = find_room_by_name(lemin, split[0]);
	room2_id = find_room_by_name(lemin, split[1]);
	
	if (room1_id == -1 || room2_id == -1)
	{
		ft_putendl_fd("ERROR", 2);
		return (-1);
	}
	
	lemin->links = realloc(lemin->links, sizeof(t_link) * (lemin->nb_links + 1));
	if (!lemin->links)
		return (-1);
	
	lemin->links[lemin->nb_links].room1_id = room1_id;
	lemin->links[lemin->nb_links].room2_id = room2_id;
	lemin->links[lemin->nb_links].capacity = 1;
	lemin->links[lemin->nb_links].flow = 0;
	
	lemin->nb_links++;
	return (0);
}

/* Parse l'entrée */
int	parse_input(t_lemin *lemin)
{
	char			*line;
	t_room_type		next_type;
	int				ret;

	lemin->nb_ants = 0;
	lemin->nb_rooms = 0;
	lemin->nb_links = 0;
	lemin->rooms = NULL;
	lemin->links = NULL;
	lemin->start_id = -1;
	lemin->end_id = -1;
	next_type = NORMAL;
	
	line = get_next_line(0);
	if (!line || sscanf(line, "%d", &lemin->nb_ants) != 1 || lemin->nb_ants <= 0)
	{
		ft_putendl_fd("ERROR", 2);
		return (-1);
	}
	free(line);
	
	while ((line = get_next_line(0)) != NULL)
	{
		if (line[0] == '\0')
		{
			free(line);
			break ;
		}
		if (line[0] == '#')
		{
			if (strcmp(line, "##start") == 0)
				next_type = START;
			else if (strcmp(line, "##end") == 0)
				next_type = END;
			free(line);
			continue ;
		}
		if (strchr(line, '-'))
		{
			ret = parse_link(lemin, line);
		}
		else
		{
			ret = parse_room(lemin, line, next_type);
			next_type = NORMAL;
		}
		free(line);
		if (ret == -1)
			return (-1);
	}
	
	if (lemin->start_id == -1 || lemin->end_id == -1)
	{
		ft_putendl_fd("ERROR", 2);
		return (-1);
	}
	
	return (0);
}

void	ft_putendl_fd(const char *s, int fd)
{
	if (s)
	{
		write(fd, s, strlen(s));
		write(fd, "\n", 1);
	}
}
