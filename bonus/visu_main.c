#include "visu.h"

/*
** Active le tour suivant en assignant de nouvelles salles cibles à toutes les fourmis
** concernées par le tour actuel, et démarre l'animation.
*/
void	trigger_next_turn(t_vars *vars)
{
	int	i;

	if (vars->current_turn >= g_nb_turns)
		return ;
	t_turn *turn = &g_turns[vars->current_turn];
	i = 0;
	while (i < turn->nb_moves)
	{
		int	ant_id = turn->moves[i].ant_id;
		int	dest_room = turn->moves[i].room_id;
		if (ant_id > 0 && ant_id <= vars->lemin.nb_ants)
		{
			vars->ants[ant_id].target_room = dest_room;
			vars->ants[ant_id].is_moving = 1;
			vars->ants[ant_id].move_progress = 0;
		}
		i++;
	}
	vars->current_turn++;
	vars->is_animating = 1;
}

/*
** Réinitialise la simulation à son état d'origine : toutes les fourmis retournent
** à la salle de départ (##start), et le compteur de tours est remis à zéro.
*/
void	reset_simulation(t_vars *vars)
{
	int	i;

	vars->current_turn = 0;
	vars->is_animating = 0;
	i = 1;
	while (i <= vars->lemin.nb_ants)
	{
		vars->ants[i].current_room = vars->lemin.start_id;
		vars->ants[i].target_room = vars->lemin.start_id;
		vars->ants[i].x = vars->room_x[vars->lemin.start_id];
		vars->ants[i].y = vars->room_y[vars->lemin.start_id];
		vars->ants[i].is_moving = 0;
		vars->ants[i].move_progress = 0;
		i++;
	}
}

/*
** Fonction de boucle principale appelée à chaque frame (loop hook).
** 1. Calcule la position des fourmis en mouvement (interpolation).
** 2. Gère l'autoplay (lecture automatique).
** 3. Redessine l'image en mémoire (double buffering).
** 4. Pousse l'image finale sur l'écran et dessine les libellés textuels des pièces.
*/
int	update_and_render(t_vars *vars)
{
	int	still_animating = 0;
	int	i = 1;

	/* 1. Mettre à jour la position des fourmis */
	while (i <= vars->lemin.nb_ants)
	{
		if (vars->ants[i].is_moving)
		{
			vars->ants[i].move_progress += 4;
			if (vars->ants[i].move_progress >= 100)
			{
				vars->ants[i].move_progress = 100;
				vars->ants[i].current_room = vars->ants[i].target_room;
				vars->ants[i].x = vars->room_x[vars->ants[i].current_room];
				vars->ants[i].y = vars->room_y[vars->ants[i].current_room];
				vars->ants[i].is_moving = 0;
			}
			else
			{
				double	t = (double)vars->ants[i].move_progress / 100.0;
				int		r_start = vars->ants[i].current_room;
				int		r_end = vars->ants[i].target_room;
				vars->ants[i].x = vars->room_x[r_start] + (vars->room_x[r_end] - vars->room_x[r_start]) * t;
				vars->ants[i].y = vars->room_y[r_start] + (vars->room_y[r_end] - vars->room_y[r_start]) * t;
				still_animating = 1;
			}
		}
		i++;
	}
	vars->is_animating = still_animating;

	/* 2. Lecture automatique (Autoplay) */
	if (!vars->is_animating && vars->autoplay)
	{
		if (vars->current_turn < g_nb_turns)
		{
			if (vars->autoplay_delay > 25)
			{
				trigger_next_turn(vars);
				vars->autoplay_delay = 0;
			}
			else
				vars->autoplay_delay++;
		}
	}

	/* 3. Rendu de l'image (Double buffering) */
	draw_bg_to_frame(&vars->frame, &vars->background_img);

	/* Dessiner les tunnels */
	i = 0;
	while (i < vars->lemin.nb_links)
	{
		int	u = vars->lemin.links[i].room1_id;
		int	v = vars->lemin.links[i].room2_id;
		draw_tunnel(&vars->frame, vars->room_x[u], vars->room_y[u], vars->room_x[v], vars->room_y[v]);
		i++;
	}

	/* Dessiner les salles */
	i = 0;
	while (i < vars->lemin.nb_rooms)
	{
		int	rx = vars->room_x[i];
		int	ry = vars->room_y[i];
		if (i == vars->lemin.start_id)
		{
			int has_ant = 0;
			int j = 1;
			while (j <= vars->lemin.nb_ants)
			{
				if (!vars->ants[j].is_moving && vars->ants[j].current_room == vars->lemin.start_id)
				{
					has_ant = 1;
					break ;
				}
				j++;
			}
			if (has_ant)
				draw_img_to_img(&vars->frame, &vars->start_ant_img, rx - 60, ry - 60, 120, 120);
			else
				draw_img_to_img(&vars->frame, &vars->start_empty_img, rx - 60, ry - 60, 120, 120);
		}
		else if (i == vars->lemin.end_id)
		{
			int has_ant = 0;
			int j = 1;
			while (j <= vars->lemin.nb_ants)
			{
				if (!vars->ants[j].is_moving && vars->ants[j].current_room == vars->lemin.end_id)
				{
					has_ant = 1;
					break ;
				}
				j++;
			}
			if (has_ant)
				draw_img_to_img(&vars->frame, &vars->end_ant_img, rx - 60, ry - 60, 120, 120);
			else
				draw_img_to_img(&vars->frame, &vars->end_empty_img, rx - 60, ry - 60, 120, 120);
		}
		else
		{
			int has_ant = 0;
			int j = 1;
			while (j <= vars->lemin.nb_ants)
			{
				if (!vars->ants[j].is_moving && vars->ants[j].current_room == i)
				{
					has_ant = 1;
					break ;
				}
				j++;
			}
			if (has_ant)
				draw_img_to_img(&vars->frame, &vars->room_ant_img, rx - 60, ry - 60, 120, 120);
			else
				draw_img_to_img(&vars->frame, &vars->room_empty_img, rx - 60, ry - 60, 120, 120);
		}
		i++;
	}

	/* Dessiner les fourmis en déplacement (sprites animés de marche) */
	i = 1;
	while (i <= vars->lemin.nb_ants)
	{
		if (vars->ants[i].is_moving)
		{
			int frame_idx = (vars->ants[i].move_progress / 5) % 4;
			draw_img_to_img(&vars->frame, &vars->ant_walk_imgs[frame_idx], (int)vars->ants[i].x - 50, (int)vars->ants[i].y - 50, 100, 100);
		}
		i++;
	}

	/* Calculer les statistiques des fourmis */
	int	ants_at_spawn = 0;
	int	ants_in_transit = 0;
	int	ants_at_end = 0;
	i = 1;
	while (i <= vars->lemin.nb_ants)
	{
		if (vars->ants[i].is_moving)
			ants_in_transit++;
		else if (vars->ants[i].current_room == vars->lemin.start_id)
			ants_at_spawn++;
		else if (vars->ants[i].current_room == vars->lemin.end_id)
			ants_at_end++;
		i++;
	}

	/* Dessiner les nombres des statistiques en haut à gauche */
	draw_large_number(&vars->frame, 400, 20, vars->lemin.nb_ants, 2, 0xFF8C00);      // Orange (Total)
	draw_large_number(&vars->frame, 400, 50, ants_at_spawn, 2, 0x2ECC71);       // Green (Départ)
	draw_large_number(&vars->frame, 400, 80, ants_in_transit, 2, 0x8B4513);     // Brown (En transit)
	draw_large_number(&vars->frame, 400, 110, ants_at_end, 2, 0xE74C3C);         // Red (Arrivée)

	/* Rendre le compteur de tour juste à droite du centre horizontal de l'image (en grand, blanc) */
	draw_large_number(&vars->frame, vars->bg_width / 2 + 80, 40, vars->current_turn, 4, 0xFFFFFF);

	/* Pousser l'image dessinée sur l'écran */
	mlx_put_image_to_window(vars->mlx, vars->win, vars->frame.ptr, 0, 0);
	return (0);
}

/*
** Gère les frappes au clavier de l'utilisateur :
** - Échap : Quitter
** - Espace : Lancer le tour suivant (si l'animation actuelle est finie)
** - R : Réinitialiser la simulation
** - A : Activer/Désactiver l'Autoplay
*/
static int	handle_keypress(int keycode, t_vars *vars)
{
	if (keycode == 65307 || keycode == 53 || keycode == 9)
	{
		mlx_destroy_window(vars->mlx, vars->win);
		exit(0);
	}
	else if (keycode == 32)
	{
		if (!vars->is_animating && vars->current_turn < g_nb_turns)
			trigger_next_turn(vars);
	}
	else if (keycode == 114 || keycode == 82)
		reset_simulation(vars);
	else if (keycode == 97 || keycode == 65)
		vars->autoplay = !vars->autoplay;
	return (0);
}

/*
** Gère l'événement de fermeture de la fenêtre graphique (croix rouge de fermeture).
*/
static int	handle_close(t_vars *vars)
{
	(void)vars;
	exit(0);
}

/*
** Point d'entrée principal du programme visualiseur.
** 1. Parse la carte et les mouvements.
** 2. Initialise la MiniLibX, charge les images XPM.
** 3. Adapte les coordonnées des salles de la carte à l'écran.
** 4. Enregistre les gestionnaires d'événements et lance la boucle de rendu.
*/
int	main(void)
{
	t_vars	vars;
	int		i;

	if (parse_visualizer_input(&vars.lemin) == -1)
	{
		free_lemin(&vars.lemin);
		ft_putendl_fd("ERROR", 2);
		return (1);
	}
	parse_moves(&vars.lemin);
	if (g_nb_turns == 0)
	{
		generate_internal_turns(&vars.lemin);
	}
	vars.mlx = mlx_init();
	if (!vars.mlx)
	{
		free_lemin(&vars.lemin);
		return (1);
	}
	vars.background_img = load_xpm_file(vars.mlx, "bonus/sprites/background.xpm");
	if (!vars.background_img.ptr)
	{
		ft_putendl_fd("Error: background sprite not found.", 2);
		free_lemin(&vars.lemin);
		return (1);
	}
	vars.bg_width = vars.background_img.width;
	vars.bg_height = vars.background_img.height;
	vars.win = mlx_new_window(vars.mlx, vars.bg_width, vars.bg_height, "Lem-in Visualizer");
	if (!vars.win)
	{
		free_lemin(&vars.lemin);
		return (1);
	}
	vars.ant_walk_imgs[0] = load_xpm_file(vars.mlx, "bonus/sprites/ant1.xpm");
	vars.ant_walk_imgs[1] = load_xpm_file(vars.mlx, "bonus/sprites/ant2.xpm");
	vars.ant_walk_imgs[2] = load_xpm_file(vars.mlx, "bonus/sprites/ant3.xpm");
	vars.ant_walk_imgs[3] = load_xpm_file(vars.mlx, "bonus/sprites/ant4.xpm");
	vars.start_empty_img = load_xpm_file(vars.mlx, "bonus/sprites/start_empty.xpm");
	vars.start_ant_img = load_xpm_file(vars.mlx, "bonus/sprites/start_ant.xpm");
	vars.end_empty_img = load_xpm_file(vars.mlx, "bonus/sprites/end_empty.xpm");
	vars.end_ant_img = load_xpm_file(vars.mlx, "bonus/sprites/end_ant.xpm");
	vars.room_empty_img = load_xpm_file(vars.mlx, "bonus/sprites/room_empty.xpm");
	vars.room_ant_img = load_xpm_file(vars.mlx, "bonus/sprites/room_ant.xpm");
	vars.frame.ptr = mlx_new_image(vars.mlx, vars.bg_width, vars.bg_height);
	vars.frame.addr = mlx_get_data_addr(vars.frame.ptr, &vars.frame.bits_per_pixel, &vars.frame.line_length, &vars.frame.endian);
	vars.frame.width = vars.bg_width;
	vars.frame.height = vars.bg_height;

	/* Mettre à l'échelle les coordonnées de la carte sur l'écran */
	vars.room_x = malloc(sizeof(int) * vars.lemin.nb_rooms);
	vars.room_y = malloc(sizeof(int) * vars.lemin.nb_rooms);
	if (vars.lemin.nb_rooms > 0)
	{
		int	min_x = vars.lemin.rooms[0].x;
		int	max_x = vars.lemin.rooms[0].x;
		int	min_y = vars.lemin.rooms[0].y;
		int	max_y = vars.lemin.rooms[0].y;
		i = 1;
		while (i < vars.lemin.nb_rooms)
		{
			if (vars.lemin.rooms[i].x < min_x)
				min_x = vars.lemin.rooms[i].x;
			if (vars.lemin.rooms[i].x > max_x)
				max_x = vars.lemin.rooms[i].x;
			if (vars.lemin.rooms[i].y < min_y)
				min_y = vars.lemin.rooms[i].y;
			if (vars.lemin.rooms[i].y > max_y)
				max_y = vars.lemin.rooms[i].y;
			i++;
		}
		i = 0;
		while (i < vars.lemin.nb_rooms)
		{
			int margin_x = 150;
			int margin_y_top = 300;
			int margin_y_bottom = 150;
			int width_range = vars.bg_width - 2 * margin_x;
			int height_range = vars.bg_height - margin_y_top - margin_y_bottom;
			if (max_x - min_x == 0)
				vars.room_x[i] = vars.bg_width / 2;
			else
				vars.room_x[i] = margin_x + (vars.lemin.rooms[i].x - min_x) * width_range / (max_x - min_x);
			if (max_y - min_y == 0)
				vars.room_y[i] = margin_y_top + height_range / 2;
			else
				vars.room_y[i] = margin_y_top + (vars.lemin.rooms[i].y - min_y) * height_range / (max_y - min_y);
			i++;
		}
	}

	/* Initialiser l'affichage des fourmis */
	vars.ants = malloc(sizeof(t_ant_display) * (vars.lemin.nb_ants + 1));
	i = 1;
	while (i <= vars.lemin.nb_ants)
	{
		vars.ants[i].id = i;
		vars.ants[i].current_room = vars.lemin.start_id;
		vars.ants[i].target_room = vars.lemin.start_id;
		vars.ants[i].x = vars.room_x[vars.lemin.start_id];
		vars.ants[i].y = vars.room_y[vars.lemin.start_id];
		vars.ants[i].is_moving = 0;
		vars.ants[i].move_progress = 0;
		i++;
	}
	vars.current_turn = 0;
	vars.is_animating = 0;
	vars.autoplay = 0;
	vars.autoplay_delay = 0;

	/* Enregistrer les crochets d'événements et démarrer la boucle */
	mlx_hook(vars.win, 2, 1L << 0, (int (*)())handle_keypress, &vars);
	mlx_hook(vars.win, 17, 0, (int (*)())handle_close, &vars);
	mlx_loop_hook(vars.mlx, (int (*)())update_and_render, &vars);
	mlx_loop(vars.mlx);

	free(vars.room_x);
	free(vars.room_y);
	free(vars.ants);
	free_lemin(&vars.lemin);
	return (0);
}
