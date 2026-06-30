# Lem-in by lengarci & clfouger

Un projet d'algorithmique et de théorie des graphes visant à acheminer des fourmis à travers une fourmilière de façon optimale.

## Présentation du projet

Le but de **Lem-in** est de trouver le chemin le plus court pour déplacer un nombre défini de fourmis de la salle `##start` à la salle `##end`, tout en évitant les embouteillages et collisions (chaque salle intermédiaire ne pouvant contenir qu'une seule fourmi à la fois).

L'algorithme de résolution repose sur :
1. **Dédoublement de nœuds** (*Node Splitting*) pour modéliser la capacité des salles (capacité = 1).
2. L'algorithme d'**Edmonds-Karp** (Ford-Fulkerson avec recherche en largeur BFS) pour trouver le flot maximum de chemins disjoints.
3. Une répartition gloutonne (*Greedy Path Assignment*) pour assigner au mieux les fourmis aux différents chemins de longueurs variables afin de minimiser le nombre de tours de jeu.

---

## Répartition de l'équipe

- **lengarci** : Algorithme et résolution du flot maximum.
- **clfouger** : Parsing, gestion d'erreurs et bonus.

---

## Compilation

Pour compiler le projet :
```bash
make bonus
```

Cela génère deux exécutables :
- `lem-in` : Le programme principal obligatoire.
- `visu-hex` : Le visualiseur graphique (Bonus utilisant la MiniLibX).

---

## Utilisation

### Lancer le programme principal
```bash
./lem-in < maps/valid/00_simple.map
```

### Lancer le visualiseur graphique (Bonus)
```bash
./visu-hex < maps/valid/00_simple.map
```
- Appuyer sur **Espace** pour faire avancer la simulation d'un tour.
- Appuyer sur **A** pour activer/désactiver le mode de lecture automatique (*Autoplay*).
- Appuyer sur **R** pour réinitialiser la simulation.
- Appuyer sur **Échap** pour quitter.