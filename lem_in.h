#ifndef LEM_IN_H
# define LEM_IN_H

# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>

typedef enum e_room_type
{
	NORMAL,
	START,
	END
}	t_room_type;

typedef struct s_room
{
	char			*name;
	int				x;
	int				y;
	t_room_type		type;
	int				id;
}	t_room;

typedef struct s_link
{
	int				room1_id;
	int				room2_id;
	int				flow;
	int				capacity;
}	t_link;

typedef struct s_path
{
	int				*rooms;
	int				length;
	int				flow;
}	t_path;

typedef struct s_lemin
{
	int				nb_ants;
	int				nb_rooms;
	int				nb_links;
	t_room			*rooms;
	t_link			*links;
	int				start_id;
	int				end_id;
	int				**adjacency;
	int				**capacity;
	int				**flow_graph;
	t_path			*paths;
	int				nb_paths;
}	t_lemin;

/* ============ PARSING ============ */

int			parse_input(t_lemin *lemin);
int			parse_room(t_lemin *lemin, char *line, t_room_type type);
int			parse_link(t_lemin *lemin, char *line);
char		*get_next_line(int fd);
char		**ft_split(char const *s, char c);

/* ============ ALGORITHM ============ */

int			compute_max_flow(t_lemin *lemin);
int			find_paths(t_lemin *lemin);
void		simulate_ants(t_lemin *lemin);
int			*bfs_find_augmenting_path(t_lemin *lemin, int *path_len);

/* ============ UTILS ============ */

void		free_lemin(t_lemin *lemin);
int			find_room_by_name(t_lemin *lemin, const char *name);
void		ft_putendl_fd(const char *s, int fd);
void		print_farm(t_lemin *lemin);

#endif
