#include "visu.h"

/* Variables globales définies ici, utilisées pour stocker les mouvements par tour */
t_turn		*g_turns = NULL;
int			g_nb_turns = 0;

/* Variables globales privées (statiques) à ce fichier pour parser la liste chaînée brute des mouvements */
static t_move_line	*g_moves = NULL;
static t_move_line	*g_moves_tail = NULL;

/* 
** Ajoute une ligne de mouvements brute (commençant par L) lue dans l'entrée standard
** à notre liste chaînée temporaire g_moves.
*/
static void	add_move_line(const char *line)
{
	t_move_line	*node;

	node = malloc(sizeof(t_move_line));
	if (!node)
		return ;
	node->line = strdup(line);
	node->next = NULL;
	if (!g_moves)
	{
		g_moves = node;
		g_moves_tail = node;
	}
	else
	{
		g_moves_tail->next = node;
		g_moves_tail = node;
	}
}

/*
** Lit la fourmilière sur l'entrée standard, en ignorant les éventuelles lignes de debug.
** Lit également les mouvements (lignes commençant par L) s'ils sont fournis par un pipe.
*/
int	parse_visualizer_input(t_lemin *lemin)
{
	char		*line;
	int			ret;
	t_room_type	next_type;

	next_type = NORMAL;
	lemin->nb_ants = 0;
	lemin->nb_rooms = 0;
	lemin->nb_links = 0;
	lemin->rooms = NULL;
	lemin->links = NULL;
	lemin->start_id = -1;
	lemin->end_id = -1;
	lemin->adjacency = NULL;
	lemin->capacity = NULL;
	lemin->flow_graph = NULL;
	lemin->paths = NULL;
	lemin->nb_paths = 0;
	line = get_next_line(0);
	if (!line)
		return (-1);
	while (line && (strncmp(line, "DEBUG:", 6) == 0 || line[0] == '#'))
	{
		free(line);
		line = get_next_line(0);
	}
	if (!line || sscanf(line, "%d", &lemin->nb_ants) != 1 || lemin->nb_ants <= 0)
	{
		if (line)
			free(line);
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
		if (strncmp(line, "DEBUG:", 6) == 0)
		{
			free(line);
			continue ;
		}
		if (line[0] == 'L')
		{
			add_move_line(line);
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
			ret = parse_link(lemin, line);
		else
		{
			ret = parse_room(lemin, line, next_type);
			next_type = NORMAL;
		}
		free(line);
		if (ret == -1)
			return (-1);
	}
	while ((line = get_next_line(0)) != NULL)
	{
		if (line[0] == 'L')
			add_move_line(line);
		free(line);
	}
	if (lemin->start_id == -1 || lemin->end_id == -1)
		return (-1);
	return (0);
}

/*
** Transforme la liste chaînée brute g_moves de lignes de mouvements textuels
** en un tableau structuré de tours et de déplacements individuels de fourmis.
*/
void	parse_moves(t_lemin *lemin)
{
	t_move_line	*curr;
	int			count;
	int			turn_idx;

	curr = g_moves;
	count = 0;
	while (curr)
	{
		count++;
		curr = curr->next;
	}
	if (count == 0)
		return ;
	g_turns = malloc(sizeof(t_turn) * count);
	g_nb_turns = count;
	curr = g_moves;
	turn_idx = 0;
	while (curr)
	{
		char	**split = ft_split(curr->line, ' ');
		int		moves_count = 0;
		while (split && split[moves_count])
			moves_count++;
		g_turns[turn_idx].nb_moves = moves_count;
		g_turns[turn_idx].moves = malloc(sizeof(t_ant_move) * moves_count);
		int i = 0;
		while (i < moves_count)
		{
			char	*p = split[i];
			if (p[0] == 'L')
			{
				int		ant_id = atoi(p + 1);
				char	*dash = strchr(p, '-');
				if (dash)
				{
					char	*room_name = dash + 1;
					int		room_id = find_room_by_name(lemin, room_name);
					g_turns[turn_idx].moves[i].ant_id = ant_id;
					g_turns[turn_idx].moves[i].room_id = room_id;
				}
			}
			free(split[i]);
			i++;
		}
		free(split);
		curr = curr->next;
		turn_idx++;
	}
}

/*
** Si aucun mouvement n'a été fourni en entrée (visualiseur lancé en autonome),
** cette fonction simule en interne l'algorithme d'Edmonds-Karp et la répartition
** gloutonne pour générer les tours et déplacements que le visualiseur animera.
*/
void	generate_internal_turns(t_lemin *lemin)
{
	int	i;

	if (compute_max_flow(lemin) == -1)
		return ;

	typedef struct s_temp_ant {
		int current_room;
		int path_idx;
		int pos_in_path;
	} t_temp_ant;

	t_temp_ant *ants = malloc(sizeof(t_temp_ant) * (lemin->nb_ants + 1));
	int *path_assignment = malloc(sizeof(int) * (lemin->nb_ants + 1));
	if (!ants || !path_assignment)
		return ;
	i = 0;
	while (i <= lemin->nb_ants)
	{
		ants[i].current_room = lemin->start_id;
		ants[i].path_idx = -1;
		ants[i].pos_in_path = 0;
		path_assignment[i] = -1;
		i++;
	}

	/* Répartition gloutonne (Greedy) identique à celle de l'exécutable principal */
	int *path_ants_count = calloc(lemin->nb_paths, sizeof(int));
	if (!path_ants_count)
	{
		free(ants);
		free(path_assignment);
		return ;
	}
	i = 1;
	while (i <= lemin->nb_ants)
	{
		if (lemin->nb_paths > 0)
		{
			int best_path = 0;
			int min_score = (lemin->paths[0].length - 1) + path_ants_count[0];
			int p = 1;
			while (p < lemin->nb_paths)
			{
				int score = (lemin->paths[p].length - 1) + path_ants_count[p];
				if (score < min_score)
				{
					min_score = score;
					best_path = p;
				}
				p++;
			}
			path_assignment[i] = best_path;
			path_ants_count[best_path]++;
		}
		i++;
	}
	free(path_ants_count);

	int ants_finished = 0;
	int turn_idx = 0;
	int max_turns = 1000;

	typedef struct s_turn_list {
		t_ant_move *moves;
		int nb_moves;
		struct s_turn_list *next;
	} t_turn_list;

	t_turn_list *head = NULL;
	t_turn_list *tail = NULL;
	int total_turns = 0;

	while (ants_finished < lemin->nb_ants && turn_idx < max_turns)
	{
		t_ant_move *temp_moves = malloc(sizeof(t_ant_move) * lemin->nb_ants);
		int moves_count = 0;
		i = 1;
		while (i <= lemin->nb_ants)
		{
			if (ants[i].current_room != lemin->end_id)
			{
				int path_idx = path_assignment[i];
				if (path_idx >= 0 && path_idx < lemin->nb_paths)
				{
					int next_room = lemin->paths[path_idx].rooms[ants[i].pos_in_path + 1];
					int can_move = 1;
					int j = 1;
					while (j <= lemin->nb_ants)
					{
						if (i != j && ants[j].current_room == next_room)
						{
							can_move = 0;
							break ;
						}
						j++;
					}
					if ((next_room == lemin->start_id || next_room == lemin->end_id) && 
						ants[i].pos_in_path + 1 > 0)
					{
						can_move = 1;
					}
					if (can_move)
					{
						ants[i].current_room = next_room;
						ants[i].pos_in_path++;
						if (ants[i].current_room == lemin->end_id)
							ants_finished++;
						temp_moves[moves_count].ant_id = i;
						temp_moves[moves_count].room_id = next_room;
						moves_count++;
					}
				}
			}
			i++;
		}
		if (moves_count > 0)
		{
			t_turn_list *node = malloc(sizeof(t_turn_list));
			node->nb_moves = moves_count;
			node->moves = malloc(sizeof(t_ant_move) * moves_count);
			memcpy(node->moves, temp_moves, sizeof(t_ant_move) * moves_count);
			node->next = NULL;
			if (!head)
			{
				head = node;
				tail = node;
			}
			else
			{
				tail->next = node;
				tail = node;
			}
			total_turns++;
		}
		free(temp_moves);
		turn_idx++;
	}
	free(ants);
	free(path_assignment);
	if (total_turns > 0)
	{
		g_turns = malloc(sizeof(t_turn) * total_turns);
		g_nb_turns = total_turns;
		t_turn_list *curr = head;
		int idx = 0;
		while (curr)
		{
			g_turns[idx].nb_moves = curr->nb_moves;
			g_turns[idx].moves = curr->moves;
			t_turn_list *next = curr->next;
			free(curr);
			curr = next;
			idx++;
		}
	}
}
