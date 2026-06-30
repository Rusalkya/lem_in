#include "../../lem_in.h"

typedef struct s_ant_state
{
	int	current_room;
	int	path_index;
	int	position_in_path;
}	t_ant_state;

/* Simule le mouvement des fourmis */
void	simulate_ants(t_lemin *lemin)
{
	t_ant_state	*ants;
	int			*path_assignment;
	int			current_turn;
	int			i;
	int			j;
	int			ants_finished;

	ants = malloc(sizeof(t_ant_state) * (lemin->nb_ants + 1));
	path_assignment = malloc(sizeof(int) * (lemin->nb_ants + 1));
	if (!ants || !path_assignment)
		return ;
	
	/* Initialiser les fourmis */
	i = 0;
	while (i <= lemin->nb_ants)
	{
		ants[i].current_room = lemin->start_id;
		ants[i].path_index = -1;
		ants[i].position_in_path = 0;
		path_assignment[i] = -1;
		i++;
	}
	
	/* Assigner les fourmis aux chemins de manière gloutonne (Greedy) */
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
	
	current_turn = 0;
	ants_finished = 0;
	
	/* Simulation des tours */
	while (ants_finished < lemin->nb_ants)
	{
		current_turn++;
		i = 1;
		while (i <= lemin->nb_ants)
		{
			if (ants[i].current_room != lemin->end_id)
			{
				int path_idx = path_assignment[i];
				if (path_idx >= 0 && path_idx < lemin->nb_paths)
				{
					int next_room = lemin->paths[path_idx].rooms[ants[i].position_in_path + 1];
					
					/* Vérifier que la room suivante n'est pas occupée */
					int can_move = 1;
					j = 1;
					while (j <= lemin->nb_ants && can_move)
					{
						if (i != j && ants[j].current_room == next_room)
							can_move = 0;
						j++;
					}
					
					/* Autoriser plusieurs fourmis en start et end */
					if ((next_room == lemin->start_id || next_room == lemin->end_id) && 
						ants[i].position_in_path + 1 > 0)
						can_move = 1;
					
					if (can_move)
					{
						ants[i].current_room = next_room;
						ants[i].position_in_path++;
						
						if (ants[i].current_room == lemin->end_id)
							ants_finished++;
						
						if (i > 1)
							printf(" ");
						printf("L%d-%s", i, lemin->rooms[ants[i].current_room].name);
					}
				}
			}
			i++;
		}
		if (current_turn > 0)
			printf("\n");
	}
	
	free(ants);
	free(path_assignment);
}
