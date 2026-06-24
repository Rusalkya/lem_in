# LEM-IN - PARTIE ALGORITHMIQUE

## Vue d'ensemble du problème

Le projet lem-in est un problème de **flot maximum** (Max Flow problem) avec une contrainte : chaque salle (sauf start et end) ne peut contenir qu'une fourmi à la fois.

Objectif : Déplacer N fourmis de START vers END en minimum de coups (tours).

---

## 1. COMPRENDRE LE PROBLÈME

### Exemple simple :
```
[0]---[2]---[3]---[1]
```

Avec 3 fourmis :
- Tour 1: L1-2, L1-3, L2-2
- Tour 2: L1-1, L2-3, L3-2
- Tour 3: L2-1, L3-3
- Tour 4: L3-1
 
**5 tours total** (4 moves + 1 pour position finale)

### Contraintes clés :
1. Une fourmi par salle (sauf start/end)
2. Une fourmi ne peut se déplacer qu'une fois par tour
3. Les chemins ne peuvent pas se croiser au même moment

---

## 2. APPROCHE : MAX FLOW

Utilisez l'algorithme de **Ford-Fulkerson** avec **BFS** (Edmonds-Karp) :

```
Pseudo-code :
1. Créer un graphe de flot (capacité = 1 pour chaque arête)
2. Tant qu'il existe un chemin augmentant de START à END :
   a. Trouver le chemin augmentant avec BFS
   b. Envoyer 1 unité de flot par ce chemin
   c. Mettre à jour le réseau résiduel
3. Le nombre de tours = (nb_ants + shortest_path_length - 1)
```

### Pourquoi ça marche ?

- Chaque unité de flot = une fourmi
- Chaque chemin dans le flot maximum = une route distincte
- Le flot force les fourmis à utiliser les meilleures combinaisons de chemins
- Aucun chevauchement car les arêtes à capacité 1 évitent les collisions

---

## 3. IMPLÉMENTATION EN C

### Étape 1 : Créer le réseau de flot

```c
typedef struct s_flow_graph {
    int **capacity;      // Capacités des arêtes
    int **flow;          // Flot actuel
    int **residual;      // Graphe résiduel
    int nb_nodes;
} t_flow_graph;

t_flow_graph *create_flow_graph(t_lemin *lemin)
{
    t_flow_graph *fg = malloc(sizeof(t_flow_graph));
    int i, j;
    
    fg->nb_nodes = lemin->nb_rooms;
    
    // Allouer les matrices (n x n)
    fg->capacity = malloc(sizeof(int*) * fg->nb_nodes);
    fg->flow = malloc(sizeof(int*) * fg->nb_nodes);
    fg->residual = malloc(sizeof(int*) * fg->nb_nodes);
    
    for (i = 0; i < fg->nb_nodes; i++) {
        fg->capacity[i] = calloc(fg->nb_nodes, sizeof(int));
        fg->flow[i] = calloc(fg->nb_nodes, sizeof(int));
        fg->residual[i] = calloc(fg->nb_nodes, sizeof(int));
    }
    
    // Initialiser les capacités
    for (i = 0; i < lemin->nb_links; i++) {
        int u = lemin->links[i].room1_id;
        int v = lemin->links[i].room2_id;
        fg->capacity[u][v] = 1;  // Arête non-dirigée
        fg->capacity[v][u] = 1;
        fg->residual[u][v] = 1;
        fg->residual[v][u] = 1;
    }
    
    return fg;
}
```

### Étape 2 : Implémenter BFS pour trouver un chemin augmentant

```c
int find_augmenting_path(int **residual, int n, int source, int sink, int *parent)
{
    int queue[n];
    int visited[n];
    int front = 0, rear = 0;
    int i;
    
    memset(visited, 0, sizeof(visited));
    memset(parent, -1, sizeof(int) * n);
    
    queue[rear++] = source;
    visited[source] = 1;
    
    while (front < rear) {
        int u = queue[front++];
        
        for (i = 0; i < n; i++) {
            if (!visited[i] && residual[u][i] > 0) {
                visited[i] = 1;
                parent[i] = u;
                queue[rear++] = i;
                
                if (i == sink)
                    return 1;  // Chemin trouvé
            }
        }
    }
    return 0;  // Pas de chemin
}
```

### Étape 3 : Implémenter Edmonds-Karp (Ford-Fulkerson + BFS)

```c
int max_flow(t_flow_graph *fg, int source, int sink)
{
    int max_flow_value = 0;
    int parent[fg->nb_nodes];
    int path_flow;
    int u, v;
    
    while (find_augmenting_path(fg->residual, fg->nb_nodes, source, sink, parent)) {
        // Trouver la capacité minimale du chemin
        path_flow = INT_MAX;
        v = sink;
        while (v != source) {
            u = parent[v];
            path_flow = min(path_flow, fg->residual[u][v]);
            v = u;
        }
        
        // Mettre à jour les capacités résiduelles
        v = sink;
        while (v != source) {
            u = parent[v];
            fg->residual[u][v] -= path_flow;
            fg->residual[v][u] += path_flow;
            fg->flow[u][v] += path_flow;
            fg->flow[v][u] -= path_flow;
            v = u;
        }
        
        max_flow_value += path_flow;
    }
    
    return max_flow_value;
}
```

---

## 4. CALCULER LE TEMPS MINIMUM

Une fois que vous avez le **flot maximum** et les **chemins utilisés** :

```c
int calculate_min_turns(t_lemin *lemin, int max_flow, int shortest_path_length)
{
    // Nombre minimal de tours =
    // (nombre de fourmis + plus court chemin - 1)
    // Cela suppose un seul chemin; avec plusieurs, c'est plus complexe.
    
    // Approche simple : simuler le mouvement
    // Pour chaque fourmi, trouver son chemin via le flot
    // Puis compter le nombre de tours jusqu'à ce que tout le monde arrive
    
    return (lemin->nb_ants + shortest_path_length - 1);
}
```

---

## 5. GÉNÉRER LA SORTIE

Une fois les chemins déterminés :

```c
void output_moves(t_lemin *lemin, t_flow_graph *fg)
{
    // Simuler mouvement tour par tour
    // Pour chaque tour :
    //   - Pour chaque fourmi :
    //     * Si elle peut se déplacer vers sa prochaine salle :
    //       - Afficher "Lant_id-room_name"
    
    // Terminer quand toutes les fourmis sont arrivées
}
```

Exemple de sortie :
```
L1-2 L2-3
L1-3 L2-4
L1-end L3-2
L2-end L3-3
L3-end
```

---

## 6. OPTIMISATIONS IMPORTANTES

### Gestion de la mémoire
- Utiliser des listes chaînées pour les arêtes (plus efficace)
- Ne pas stocker de matrice N×N si N est grand (4000 rooms)
- Utiliser une liste d'adjacence à la place

### Complexité
- BFS : O(V + E)
- Ford-Fulkerson avec Edmonds-Karp : O(V × E²)
- Total pour ce problème : O(V² × E) généralement acceptable

### Améliorations
- Utiliser **Dinic's algorithm** (O(V² × E)) si trop lent
- Implémenter un **shortest path cache**
- Préparer les chemins en parallèle quand possible

---

## 7. PSEUDO-CODE FINAL

```
MAIN:
  1. Parser l'entrée (déjà fait!)
  2. Créer le graphe de flot
  3. Calculer le max flow (Edmonds-Karp)
  4. Extraire les chemins du flot
  5. Assigner les fourmis aux chemins
  6. Simuler et afficher mouvement par mouvement

TERMINER quand toutes les fourmis sont au END
```

---

## 8. EXEMPLE DÉTAILLÉ

Input :
```
3
##start
0 0 0
##end
1 5 0
0-1
```

- Graphe : 0 (start) → 1 (end)
- 3 fourmis, 1 seul chemin
- Max flow = 1 (capacité du seul lien)

Mais avec 3 fourmis, chacune doit attendre son tour :
- Tour 1: L1-1
- Tour 2: L2-1, (L1-1 est arrivée)
- Tour 3: L3-1, (L2-1 est arrivée)

(5 tours total)

---

## Fichiers à implémenter

1. `srcs/algorithm/flow.c` - Max flow
2. `srcs/algorithm/paths.c` - Extraction des chemins
3. `srcs/algorithm/simulation.c` - Simulation et output

Bonne chance! 🚀
