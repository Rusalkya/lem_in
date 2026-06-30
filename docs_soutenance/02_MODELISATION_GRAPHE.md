# 2. Modélisation : de la fourmilière au réseau de flot

C'est **le concept le plus malin du projet** et un point sur lequel les examinateurs adorent
appuyer. Maîtrise-le à fond.

---

## 2.1 Le problème : capacité sur les SOMMETS, pas sur les arêtes

Le flot maximum « classique » limite le débit sur les **arêtes**. Or notre contrainte
principale (« une fourmi par salle ») est une limite sur les **sommets** (les salles).

Comment imposer une capacité **sur un sommet** quand l'algorithme ne sait gérer que les
arêtes ? → **On dédouble le sommet.**

---

## 2.2 Le dédoublement de nœuds (*Node Splitting*)

Chaque salle `i` est transformée en **deux nœuds** :

- un nœud **IN**  d'indice `2*i`     (toutes les arêtes *entrantes* arrivent ici) ;
- un nœud **OUT** d'indice `2*i + 1` (toutes les arêtes *sortantes* partent d'ici) ;
- reliés par une **arête interne** `IN → OUT` de **capacité 1**.

```
        salle i                         après dédoublement
                                   ┌──────────────────────────┐
   ──→ [ salle i ] ──→     ⟹      ──→ (2i)IN ──cap=1──> (2i+1)OUT ──→
                                   └──────────────────────────┘
```

**Pourquoi ça marche ?** Tout chemin qui « traverse » la salle `i` doit emprunter l'arête
interne `IN→OUT`. Comme elle a une **capacité de 1**, **au plus une unité de flot** (donc
au plus une fourmi) peut traverser la salle. La contrainte de sommet est devenue une
contrainte d'arête. 🎯

### Cas particulier : start et end

Pour `start` et `end`, on ne veut **pas** limiter à 1 (elles peuvent contenir toutes les
fourmis). Donc leur arête interne a une capacité de `nb_ants` :

```c
// flow.c:103-106
int capacity = 1;
if (i == lemin->start_id || i == lemin->end_id)
    capacity = lemin->nb_ants;
add_directed_edge(fg, 2 * i, 2 * i + 1, capacity);
```

> 💡 Subtilité que tu peux sortir pour briller : la **source** du flot est `2*start+1`
> (le OUT de start) et le **puits** est `2*end` (le IN de end). Du coup l'arête interne de
> start (en amont de la source) et celle de end (en aval du puits) **ne sont jamais
> traversées** par un chemin source→puits. Leur capacité `nb_ants` est donc inoffensive — la
> vraie limite vient du **degré** de start/end (nb de tunnels voisins).

---

## 2.3 Les tunnels (links)

Un tunnel `u-v` est **bidirectionnel**. On le traduit par **deux arêtes dirigées** entre les
nœuds dédoublés, chacune de capacité 1 :

```c
// flow.c:120-121
add_directed_edge(fg, 2*u + 1, 2*v, 1);   // sortir de u  → entrer dans v
add_directed_edge(fg, 2*v + 1, 2*u, 1);   // sortir de v  → entrer dans u
```

C'est cohérent : on quitte une salle par son **OUT** et on entre dans une autre par son **IN**.

### Schéma complet pour un tunnel `u-v`

```
 (2u)IN ─cap1→ (2u+1)OUT ─cap1→ (2v)IN ─cap1→ (2v+1)OUT
       ▲                                              │
       └──────────────── cap1 ────────────────────────┘   (sens v→u)
```

---

## 2.4 Les structures de données (listes d'adjacence)

Définies en haut de [flow.c](../srcs/algorithm/flow.c) :

```c
typedef struct s_edge {
    int to;             // nœud destination
    int capacity;       // capacité de l'arête
    int flow;           // flot actuel qui y circule
    int rev_edge_idx;   // index de l'arête INVERSE chez le nœud 'to'
} t_edge;

typedef struct s_node {
    t_edge *edges;      // tableau dynamique d'arêtes sortantes
    int     edge_count;
    int     edge_capacity;
} t_node;

typedef struct s_flow_graph {
    t_node *nodes;
    int     nb_nodes;   // = 2 * nb_rooms
} t_flow_graph;
```

### Pourquoi une liste d'adjacence et pas une matrice N×N ?

C'est **la** question piège (parce que le brouillon `ALGORITHME.md` parle de matrices).
Réponse :

- L'énoncé autorise jusqu'à **plusieurs milliers de salles** (la map `05_thousand_ants`
  l'illustre). Une matrice `N×N` coûterait `O(N²)` en mémoire → des **millions d'entiers**,
  voire des milliards. Avec dédoublement c'est `(2N)²`, encore pire.
- Une fourmilière est un graphe **creux** (peu d'arêtes par salle). Une liste d'adjacence
  coûte `O(V + E)` en mémoire et parcourt seulement les arêtes existantes.

→ **Liste d'adjacence = choix correct pour de gros graphes creux.**

---

## 2.5 Le réseau résiduel et les arêtes inverses

C'est le mécanisme **central** de Ford-Fulkerson. À retenir absolument.

Quand on ajoute une arête `u → v` de capacité `c`, on ajoute **AUSSI une arête inverse**
`v → u` de capacité **0** :

```c
// add_directed_edge (flow.c:48-56)
// arête directe u → v
u->edges[...] = { .to=v, .capacity=c, .flow=0, .rev_edge_idx = (index chez v) };
// arête inverse v → u  (capacité 0 !)
v->edges[...] = { .to=u, .capacity=0, .flow=0, .rev_edge_idx = (index chez u) };
```

Le champ `rev_edge_idx` permet, depuis n'importe quelle arête, de retrouver **instantanément**
son arête inverse pour mettre à jour les deux flots ensemble.

### À quoi sert la capacité résiduelle ?

La **capacité résiduelle** d'une arête = `capacity - flow`. C'est « combien on peut encore
pousser ».

- Sur l'arête **directe** : `c - flow` (au départ `c`).
- Sur l'arête **inverse** : `0 - flow`. Quand on a poussé `f` unités sur la directe
  (`flow = +f`), l'inverse a `flow = -f`, donc sa résiduelle vaut `0 - (-f) = f`.

➡️ **Pousser du flot sur l'arête inverse = annuler du flot déjà placé sur la directe.**
C'est ce qui permet à l'algorithme de **« revenir sur ses décisions »** et de rerouter les
fourmis pour atteindre l'optimum. Sans arêtes inverses, Ford-Fulkerson resterait bloqué sur
des choix sous-optimaux.

> Métaphore : si une première fourmi a réservé un tunnel mais qu'une meilleure combinaison
> globale nécessite qu'elle « rende » ce tunnel, l'arête inverse lui permet de faire marche
> arrière. (Voir l'exemple détaillé dans [03_EDMONDS_KARP.md](03_EDMONDS_KARP.md).)

---

## 2.6 Récap visuel : nombre de nœuds et indices

Pour `nb_rooms` salles :
- `nb_nodes = 2 * nb_rooms`
- salle `i` → IN = `2*i`, OUT = `2*i + 1`
- **source** (départ du flot) = `2 * start_id + 1` (OUT de start)
- **puits** (arrivée du flot)  = `2 * end_id`     (IN de end)

➡️ Suite : [03_EDMONDS_KARP.md](03_EDMONDS_KARP.md) — l'algorithme qui calcule le flot maximum.
