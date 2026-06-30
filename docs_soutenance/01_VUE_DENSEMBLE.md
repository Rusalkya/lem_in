# 1. Vue d'ensemble — Quel problème résout-on ?

## 1.1 L'énoncé en clair

On reçoit en entrée :
- un **nombre de fourmis** `N` ;
- une **fourmilière** = un ensemble de **salles** (rooms) et de **tunnels** (links) ;
- deux salles spéciales : `##start` (départ) et `##end` (arrivée).

On doit **déplacer les N fourmis de `start` à `end` en un nombre minimum de tours**, en respectant :

1. **Une salle intermédiaire ne contient qu'une fourmi à la fois.**
   (`start` et `end` peuvent en contenir autant qu'on veut.)
2. **Un tunnel ne peut être emprunté que par une fourmi par tour** (pas de croisement).
3. À chaque tour, **chaque fourmi peut avancer d'au plus une salle**, et **toutes les fourmis bougent simultanément**.

La sortie est la fourmilière, puis une ligne par tour listant les déplacements sous la
forme `Lx-nom_salle` (fourmi numéro `x` va dans la salle `nom_salle`).

### Exemple minimal

```
Entrée (maps/valid/00_simple.map) :     Graphe :
3                                         start(0) ─ 2 ─ 3 ─ end(1)
##start
0 1 0          (salle "0", coords 1,0)
##end
1 5 0          (salle "1")
2 9 0
3 13 0
0-2
2-3
3-1
```

Un seul chemin `0→2→3→1` de longueur 3. Avec 3 fourmis, on les fait entrer en file
indienne : il faut **5 tours** (3 arêtes à traverser + 2 tours de décalage pour les 2 fourmis
suivantes).

---

## 1.2 La clé : ce n'est PAS un simple « plus court chemin »

Le réflexe naïf serait : « je calcule le plus court chemin et j'y envoie toutes les fourmis ».
**C'est faux**, pour deux raisons :

- Comme **une seule fourmi par salle**, un chemin se comporte comme un **tuyau qui débite
  1 fourmi par tour**. Avec beaucoup de fourmis, un seul chemin devient un goulot.
- S'il existe **plusieurs chemins disjoints** (qui ne partagent aucune salle intermédiaire),
  on peut faire circuler les fourmis **en parallèle** → beaucoup moins de tours.

Donc le vrai problème est : **« combien de chemins parallèles puis-je faire passer
simultanément, et lesquels ? »** → c'est exactement la question du **flot maximum**.

### Intuition « plomberie »

Imagine de l'eau qui coule de `start` à `end`. Chaque tunnel laisse passer **1 unité de
débit** (1 fourmi/tour). On veut le **débit total maximum**. Ce débit max = le nombre de
fourmis qu'on peut faire avancer **en même temps** à chaque tour, une fois le pipeline rempli.

---

## 1.3 Pourquoi « flot maximum » et pas autre chose ?

| Caractéristique du problème | Notion de flot correspondante |
|-----------------------------|-------------------------------|
| 1 fourmi = 1 objet à acheminer | 1 unité de flot |
| 1 tunnel = passage pour 1 fourmi/tour | arête de **capacité 1** |
| 1 salle = 1 fourmi max | **capacité de sommet 1** (→ dédoublement de nœud, cf. fichier 02) |
| chemins parallèles | **chemins disjoints** d'un flot entier |
| nombre de fourmis simultanées | **valeur du flot maximum** |

Le **flot maximum** nous donne donc directement le nombre maximal de chemins parallèles
utilisables, **et** (en décomposant le flot) les chemins eux-mêmes.

> ⚠️ Nuance importante à connaître (cf. fichier 06) : le flot **maximum** ne garantit pas
> **à lui seul** le nombre de tours minimal. Parfois, utiliser *moins* de chemins mais plus
> courts donne moins de tours. Notre implémentation prend les chemins du flot max puis
> **répartit les fourmis de façon gloutonne** pour absorber cette subtilité (fichier 05).

---

## 1.4 Le pipeline complet du programme

```
        ┌─────────────┐   ┌────────────────────┐   ┌─────────────────┐   ┌──────────────┐
 stdin →│   PARSING   │ → │  GRAPHE DE FLOT     │ → │  EDMONDS-KARP    │ → │  EXTRACTION  │
        │ (collègue)  │   │ create_flow_graph   │   │  (flot maximum)  │   │  des chemins │
        └─────────────┘   └────────────────────┘   └─────────────────┘   └──────┬───────┘
                                                                                  │
                                                              ┌───────────────────▼────────┐
                                                stdout  ←──── │  SIMULATION tour par tour   │
                                                              │  (répartition gloutonne)    │
                                                              └─────────────────────────────┘
```

Orchestré par [main.c](../srcs/main.c) :
```c
parse_input(&lemin);            // collègue
compute_max_flow(&lemin);       // flow.c  → construit graphe, Edmonds-Karp, extract_paths
simulate_ants(&lemin);          // simulate.c → répartit + affiche
```

**Ta partie = les deux dernières grandes boîtes** (`compute_max_flow` et `simulate_ants`).

➡️ Suite : [02_MODELISATION_GRAPHE.md](02_MODELISATION_GRAPHE.md) — comment on transforme la
fourmilière en réseau de flot.
