# 📚 Documentation de soutenance — LEM-IN (partie algorithmique)

> But de ce dossier : te permettre de **défendre intégralement la partie algo** du projet
> en soutenance, même sans avoir écrit le code toi-même.
> Ta collègue défend le **parsing** et les **bonus** (visualiseur MiniLibX) ; toi tu portes
> tout ce qui se passe **après** que la fourmilière a été lue en mémoire.

---

## Comment utiliser ce dossier

Lis dans l'ordre. Chaque fichier est autonome mais ils s'enchaînent logiquement.

| # | Fichier | Ce que tu sauras défendre après l'avoir lu |
|---|---------|---------------------------------------------|
| 0 | **00_INDEX.md** (ce fichier) | Le pitch 30 s, le plan, où se trouve chaque morceau de code |
| 1 | [01_VUE_DENSEMBLE.md](01_VUE_DENSEMBLE.md) | Quel est le problème, pourquoi c'est un problème de **flot maximum** |
| 2 | [02_MODELISATION_GRAPHE.md](02_MODELISATION_GRAPHE.md) | Le **dédoublement de nœuds**, les structures de données, le réseau résiduel |
| 3 | [03_EDMONDS_KARP.md](03_EDMONDS_KARP.md) | **Le cœur** : Ford-Fulkerson + BFS, chemin augmentant, arêtes inverses |
| 4 | [04_EXTRACTION_CHEMINS.md](04_EXTRACTION_CHEMINS.md) | Comment on reconstruit les chemins concrets à partir du flot |
| 5 | [05_SIMULATION.md](05_SIMULATION.md) | Répartition **gloutonne** des fourmis + simulation tour par tour |
| 6 | [06_COMPLEXITE_ET_PREUVES.md](06_COMPLEXITE_ET_PREUVES.md) | Complexité, terminaison, théorème **flot-max / coupe-min** |
| 7 | [07_QUESTIONS_SOUTENANCE.md](07_QUESTIONS_SOUTENANCE.md) | Q/R anticipées + **les défauts connus à assumer franchement** |
| 8 | [08_GLOSSAIRE.md](08_GLOSSAIRE.md) | Tous les termes (flot, résiduel, augmentant, coupe…) en une ligne |

---

## ⏱️ Le pitch en 30 secondes (à savoir réciter)

> « Lem-in, c'est faire passer N fourmis de `##start` à `##end` en un **minimum de tours**,
> sachant qu'une salle intermédiaire ne peut contenir **qu'une seule fourmi à la fois**.
>
> On modélise la fourmilière comme un **graphe**, et on traite ça comme un problème de
> **flot maximum**. La contrainte “une fourmi par salle” est encodée par un
> **dédoublement de chaque salle en deux nœuds** reliés par une arête de capacité 1.
>
> On calcule le flot maximum avec **Edmonds-Karp** (Ford-Fulkerson où le chemin augmentant
> est trouvé par **BFS**). Chaque unité de flot correspond à un **chemin disjoint** de start
> à end. On extrait ces chemins, puis on **répartit les fourmis** dessus de façon gloutonne
> pour équilibrer la charge, et enfin on **simule** le déplacement tour par tour. »

Si tu ne devais retenir qu'une phrase : **« 1 fourmi = 1 unité de flot ; le dédoublement de nœuds impose 1 fourmi par salle. »**

---

## 🗺️ Carte du code (où est quoi)

```
srcs/
├── main.c                  → orchestration : parse → max_flow → simulate
├── parsing.c               → (collègue) lecture de la fourmilière
├── utils.c                 → get_next_line, ft_split, print_farm, free
└── algorithm/
    ├── flow.c              → ⭐ TON CŒUR : graphe de flot + Edmonds-Karp + extraction
    └── simulate.c          → ⭐ répartition gloutonne + simulation tour par tour
lem_in.h                    → toutes les structures (t_room, t_link, t_path, t_lemin)
```

### Les fonctions clés que tu dois pouvoir pointer du doigt

| Fonction | Fichier:ligne | Rôle |
|----------|---------------|------|
| `create_flow_graph` | [flow.c:83](../srcs/algorithm/flow.c) | Construit le réseau de flot (dédoublement + tunnels) |
| `add_directed_edge` | [flow.c:24](../srcs/algorithm/flow.c) | Ajoute une arête + son arête inverse (résiduel) |
| `find_augmenting_path` | [flow.c:131](../srcs/algorithm/flow.c) | BFS qui cherche un chemin start→end dans le résiduel |
| `edmonds_karp` | [flow.c:183](../srcs/algorithm/flow.c) | Boucle Ford-Fulkerson : augmente le flot tant qu'il y a un chemin |
| `extract_paths` | [flow.c:233](../srcs/algorithm/flow.c) | Reconstruit les chemins concrets en suivant le flot |
| `compute_max_flow` | [flow.c:316](../srcs/algorithm/flow.c) | Chef d'orchestre de l'algo de flot |
| `simulate_ants` | [simulate.c:11](../srcs/algorithm/simulate.c) | Répartit les fourmis + affiche les mouvements |

---

## ⚠️ Avertissement honnête (lis 07_QUESTIONS avant la soutenance)

Le fichier `ALGORITHME.md` à la racine du projet décrit une approche **par matrices
d'adjacence**. **Ce n'est PAS ce que fait le vrai code** : c'était un brouillon / des notes.
Le code réel utilise des **listes d'arêtes** (plus efficace). Si un examinateur te montre
`ALGORITHME.md`, dis que c'est un document de conception préliminaire et que
l'implémentation finale a retenu les listes d'adjacence — explique pourquoi (voir
[02_MODELISATION_GRAPHE.md](02_MODELISATION_GRAPHE.md)).

Il y a aussi **deux petits défauts** dans le code livré (un `printf("DEBUG…")` et un
espace parasite en début de ligne). Ils sont expliqués et assumés dans
[07_QUESTIONS_SOUTENANCE.md](07_QUESTIONS_SOUTENANCE.md) — **ne te fais pas surprendre dessus.**
