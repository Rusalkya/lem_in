# 4. Extraction des chemins à partir du flot

À la fin d'Edmonds-Karp, on a un **flot** : des valeurs `flow` posées sur les arêtes. Mais
pour déplacer les fourmis, il nous faut des **chemins explicites** : des listes ordonnées de
salles `start → … → end`. C'est le rôle de [`extract_paths`](../srcs/algorithm/flow.c)
(flow.c:233).

---

## 4.1 Principe : « suivre le flot »

Un flot maximum entier sur ce graphe se **décompose en chemins disjoints** de la source au
puits. Pour les retrouver, on part de start et on **suit les arêtes qui portent du flot**
(`flow > 0`).

```c
int *path_rooms = ...;
int path_len = 0;
path_rooms[path_len++] = lemin->start_id;     // tout chemin commence par start

int current = e->to;                          // 1ère arête sortant de start avec flow > 0
int current_room = current / 2;               // nœud → salle :  nœud/2 = id de la salle

while (current_room != lemin->end_id) {
    path_rooms[path_len++] = current_room;     // on enregistre la salle traversée
    int out_node = current + 1;                // on passe du IN (2i) au OUT (2i+1)
    // on cherche l'unique arête sortante de OUT qui porte du flot
    for (chaque arête e_out de out_node)
        if (e_out->flow > 0) { next_node = e_out->to; break; }
    current = next_node;
    current_room = current / 2;
}
path_rooms[path_len++] = lemin->end_id;        // on termine par end
```

### Les deux astuces d'indices à savoir expliquer

- **`current / 2` = id de la salle.** Comme salle `i` → nœuds `2i` (IN) et `2i+1` (OUT), une
  division entière par 2 redonne `i` dans les deux cas.
- **`current + 1` = passer du IN au OUT** de la même salle. On entre dans une salle par son
  IN (`2i`), et pour continuer on repart de son OUT (`2i+1 = 2i + 1`).

---

## 4.2 Pourquoi un seul `flow > 0` en sortie de chaque salle ?

Parce que chaque salle intermédiaire a une **capacité interne de 1** : au plus 1 unité de
flot la traverse. Donc en sortie d'une salle interne, il y a **exactement une** arête avec
`flow > 0`. On peut suivre ce flot **sans ambiguïté** jusqu'à end. 🎯

Au niveau de start (`start_out`, flow.c:250), il peut y avoir **plusieurs** arêtes avec
`flow > 0` : une par chemin disjoint. La boucle externe (flow.c:252) démarre donc **un chemin
par arête sortante de start qui porte du flot**.

---

## 4.3 Stockage des chemins

Chaque chemin trouvé est rangé dans `lemin->paths[]` (type `t_path`, défini dans
[lem_in.h:33](../lem_in.h)) :

```c
typedef struct s_path {
    int *rooms;    // tableau des ids de salles, de start à end
    int  length;   // nombre de salles dans le chemin (start et end compris)
    int  flow;     // 1 (chaque chemin porte 1 unité)
} t_path;
```

`lemin->nb_paths` = nombre de chemins extraits = valeur du flot maximum.

> Détail défensif : `lemin->paths` est alloué pour `nb_rooms + 1` chemins (flow.c:243),
> majorant large et sûr du nombre de chemins disjoints possibles.

---

## 4.4 Exemple concret

Sur `02_two_paths.map`, après le flot max = 2, `extract_paths` produit :

```
paths[0] = { rooms = [start, A1, end],     length = 3, flow = 1 }
paths[1] = { rooms = [start, B1, B2, end], length = 4, flow = 1 }
```

Ces deux listes sont **disjointes** (aucune salle intermédiaire commune) — garanti par la
capacité 1 des salles. C'est exactement ce dont la simulation a besoin : des couloirs
indépendants où faire circuler les fourmis sans collision.

---

## 4.5 Et `find_paths` alors ?

Tu remarqueras une fonction `find_paths` (flow.c:344) qui ne fait rien (`return 0`). C'est un
**vestige** déclaré dans le `.h` mais **jamais utilisé** : c'est `extract_paths`, appelée
depuis `compute_max_flow`, qui fait réellement le travail. À mentionner honnêtement si on te
pose la question (voir [07_QUESTIONS_SOUTENANCE.md](07_QUESTIONS_SOUTENANCE.md)).

➡️ Suite : [05_SIMULATION.md](05_SIMULATION.md) — répartir les fourmis sur ces chemins et
afficher les tours.
