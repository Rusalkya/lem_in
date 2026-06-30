# 3. Le cœur : Edmonds-Karp (flot maximum)

C'est **le morceau le plus important** de ta soutenance. Prends le temps de pouvoir le
dérouler au tableau.

---

## 3.1 Ford-Fulkerson en une phrase

> **Tant qu'il existe un chemin de la source au puits dans le réseau résiduel (un « chemin
> augmentant »), on pousse autant de flot que possible dessus, et on recommence.**

**Edmonds-Karp = Ford-Fulkerson où le chemin augmentant est cherché par BFS** (parcours en
largeur). Le choix du BFS (plutôt que DFS) est ce qui rend la complexité **polynomiale** et
garantie (cf. fichier 06).

---

## 3.2 Étape A — Trouver un chemin augmentant par BFS

Fonction [`find_augmenting_path`](../srcs/algorithm/flow.c) (flow.c:131).

```c
int find_augmenting_path(t_flow_graph *fg, int source, int sink,
                         int *parent_node, int *parent_edge)
{
    // file BFS + tableau visited
    queue[rear++] = source; visited[source] = 1;
    while (front < rear) {
        u = queue[front++];
        if (u == sink) return 1;                 // puits atteint → chemin trouvé
        for (chaque arête e sortant de u) {
            int residual = e->capacity - e->flow; // capacité résiduelle
            if (!visited[e->to] && residual > 0) {
                visited[e->to] = 1;
                parent_node[e->to] = u;           // d'où on vient
                parent_edge[e->to] = i;           // par quelle arête
                queue[rear++] = e->to;
            }
        }
    }
    return 0;  // plus aucun chemin → flot maximum atteint
}
```

Points à expliquer :
- On ne suit une arête que si sa **résiduelle est > 0** (`capacity - flow > 0`) : c'est ce
  qui fait qu'on explore le **réseau résiduel** et pas le graphe brut.
- On mémorise pour chaque nœud **par où on est arrivé** : `parent_node[v]` (le nœud
  précédent) et `parent_edge[v]` (l'index de l'arête empruntée). Ça permet de
  **remonter le chemin** ensuite.
- BFS ⇒ on trouve le chemin augmentant **avec le moins d'arêtes** (le plus court en nombre
  de sauts). C'est l'ingrédient clé d'Edmonds-Karp.

---

## 3.3 Étape B — Pousser le flot le long du chemin

Fonction [`edmonds_karp`](../srcs/algorithm/flow.c) (flow.c:183).

```c
while (find_augmenting_path(fg, source, sink, parent_node, parent_edge)) {

    // 1) Trouver le goulot : la résiduelle minimale le long du chemin
    int path_flow = 1000000;
    for (v = sink; v != source; v = parent_node[v]) {
        t_edge *e = &fg->nodes[parent_node[v]].edges[parent_edge[v]];
        int residual = e->capacity - e->flow;
        if (residual < path_flow) path_flow = residual;
    }

    // 2) Mettre à jour le flot : +path_flow sur la directe, -path_flow sur l'inverse
    for (v = sink; v != source; v = parent_node[v]) {
        t_edge *e   = &fg->nodes[parent_node[v]].edges[parent_edge[v]];
        t_edge *rev = &fg->nodes[v].edges[e->rev_edge_idx];
        e->flow   += path_flow;
        rev->flow -= path_flow;
    }
    max_flow_value += path_flow;
}
return max_flow_value;
```

Deux phases :
1. **Goulot d'étranglement** (`path_flow`) : on remonte du puits à la source en suivant
   `parent_node`/`parent_edge`, on prend le **minimum** des capacités résiduelles. Ici, comme
   presque toutes les arêtes ont une capacité 1, `path_flow` vaut quasi toujours **1**
   → chaque itération ajoute **un chemin / une fourmi**.
2. **Mise à jour** : sur chaque arête du chemin, `flow += path_flow` ; sur son **inverse**,
   `flow -= path_flow` (grâce à `rev_edge_idx`). C'est ce `-=` qui ouvre la possibilité de
   **reroutage** (cf. §3.5).

À la fin, `max_flow_value` = **valeur du flot maximum** = nombre de chemins disjoints
trouvés = nombre maximal de fourmis qui peuvent avancer en parallèle.

---

## 3.4 Déroulé visuel sur une map à deux chemins

Map `maps/valid/02_two_paths.map` (simplifiée) :

```
            A1
          ↗    ↘
    start         end
          ↘    ↗
            B1 → B2
```
Tunnels : `start-A1`, `A1-end`, `start-B1`, `B1-B2`, `B2-end`.

- **Itération 1** : BFS trouve le plus court chemin `start → A1 → end` (2 arêtes après
  dédoublement compté en salles). On pousse 1 unité. Les arêtes internes de A1 sont
  saturées (`flow=1`, résiduelle=0).
- **Itération 2** : BFS ne peut plus passer par A1 (saturé). Il trouve
  `start → B1 → B2 → end`. On pousse 1 unité.
- **Itération 3** : plus aucun chemin augmentant (start n'a que 2 voisins, tous deux
  saturés). BFS renvoie 0. **Flot max = 2.**

Résultat : **2 chemins disjoints** → on peut faire avancer 2 fourmis par tour.

---

## 3.5 Le rôle des arêtes inverses (l'exemple qui fait la différence)

Question piège classique : *« à quoi servent vraiment les arêtes inverses ? »*

Cas où Ford-Fulkerson ferait un mauvais choix au début :

```
        ┌──→ X ──┐
   start          ──→ Y ──→ end          (arête centrale X→Y partagée)
        └──→ ... ─┘
```

Si la première augmentation « gaspille » l'arête centrale partagée, un algorithme sans retour
arrière resterait coincé à un flot de 1. Grâce à l'**arête inverse**, une augmentation
ultérieure peut emprunter `Y → X` (la résiduelle de l'inverse est devenue > 0), ce qui
**annule** le passage initial sur l'arête centrale et **redirige** les deux flots
correctement → flot de 2.

> Formulation simple en soutenance : *« l'arête inverse, c'est le bouton “annuler” :
> elle permet à l'algo de défaire un placement de flot devenu sous-optimal, pour réorganiser
> les chemins et atteindre le vrai maximum. Sans ça, Ford-Fulkerson ne serait pas correct. »*

---

## 3.6 Chef d'orchestre : `compute_max_flow`

[flow.c:316](../srcs/algorithm/flow.c) — assemble tout :

```c
int compute_max_flow(t_lemin *lemin) {
    t_flow_graph *fg = create_flow_graph(lemin);   // §2 : dédoublement + tunnels
    int source = 2 * lemin->start_id + 1;           // OUT de start
    int sink   = 2 * lemin->end_id;                 // IN  de end
    int max_flow = edmonds_karp(fg, source, sink);  // §3 : flot maximum
    extract_paths(lemin, fg);                       // fichier 04 : reconstruire les chemins
    free_flow_graph(fg);
    return max_flow;
}
```

Si `max_flow <= 0`, c'est qu'il **n'existe aucun chemin** de start à end → [main.c](../srcs/main.c)
affiche `ERROR` (cas de la map invalide `09_no_path.map`).

➡️ Suite : [04_EXTRACTION_CHEMINS.md](04_EXTRACTION_CHEMINS.md) — comment passer du « flot »
(des nombres sur des arêtes) à des **listes de salles concrètes**.
