#include "lem_in.h"

int	main(void)
{
	t_lemin	lemin;
	int		max_flow;

	if (parse_input(&lemin) == -1)
	{
		free_lemin(&lemin);
		return (1);
	}
	
	printf("DEBUG: nb_rooms=%d, start_id=%d, end_id=%d\n", lemin.nb_rooms, lemin.start_id, lemin.end_id);
	
	/* Afficher la ferme */
	print_farm(&lemin);
	

	/* Calculer le max flow */
	max_flow = compute_max_flow(&lemin);
	if (max_flow <= 0)
	{
		free_lemin(&lemin);
		ft_putendl_fd("ERROR", 2);
		return (1);
	}
	
	/* Simuler les fourmis */
	simulate_ants(&lemin);
	
	free_lemin(&lemin);
	return (0);
}
