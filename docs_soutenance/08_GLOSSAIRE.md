# 8. Glossaire — tous les termes en une ligne

À relire juste avant d'entrer en soutenance.

| Terme | Définition courte |
|-------|-------------------|
| **Graphe** | Ensemble de **sommets** (salles) reliés par des **arêtes** (tunnels). |
| **Flot (flow)** | Quantité qui circule sur une arête, bornée par sa capacité. Ici : des fourmis. |
| **Capacité** | Débit maximal d'une arête. Tunnels = 1 ; arête interne d'une salle = 1 (start/end = nb_ants). |
| **Réseau de flot** | Graphe dirigé avec une **source**, un **puits**, et des capacités sur les arêtes. |
| **Source / Puits** | Origine (`2*start+1`) et destination (`2*end`) du flot. |
| **Flot maximum** | Le plus grand débit possible de la source au puits. = nb max de chemins parallèles. |
| **Chemin augmentant** | Chemin source→puits où **toutes** les arêtes ont une capacité résiduelle > 0. |
| **Capacité résiduelle** | `capacity - flow` : ce qu'on peut encore pousser sur une arête. |
| **Réseau résiduel** | Le graphe des capacités résiduelles, sur lequel le BFS cherche un chemin augmentant. |
| **Arête inverse** | Arête « miroir » de capacité initiale 0 ; pousser dessus = **annuler** du flot (reroutage). |
| **Goulot (bottleneck)** | Capacité résiduelle **minimale** le long d'un chemin augmentant = flot qu'on y pousse. |
| **Ford-Fulkerson** | Méthode : augmenter le flot tant qu'il existe un chemin augmentant. |
| **Edmonds-Karp** | Ford-Fulkerson où le chemin augmentant est trouvé par **BFS** → `O(V·E²)` garanti. |
| **BFS** | Parcours en largeur ; trouve le plus court chemin en nombre d'arêtes. |
| **Dédoublement de nœuds** | Découper une salle en IN (`2i`) + OUT (`2i+1`) reliés par une arête cap. 1 pour limiter la salle à 1 fourmi. |
| **Chemins disjoints (en sommets)** | Chemins ne partageant aucune salle intermédiaire → pas de collision. |
| **Liste d'adjacence** | Représentation du graphe stockant, par nœud, la liste de ses arêtes. Mémoire `O(V+E)`. |
| **Théorème flot-max / coupe-min** | Flot max = capacité de la plus petite coupe séparant source et puits. |
| **Coupe (cut)** | Partition des sommets séparant source et puits ; sa capacité = somme des arêtes traversées. |
| **Théorème de Menger** | Nb max de chemins disjoints = taille min d'un ensemble séparant source et puits. |
| **Répartition gloutonne** | Assigner chaque fourmi au chemin de score `(longueur-1)+fourmis_déjà_dessus` minimal. |
| **Tour (turn)** | Une étape de simulation : chaque fourmi avance d'au plus une salle, simultanément. |

---

## Les 3 indices à ne jamais confondre

| Objet | Formule | Exemple (salle id=3) |
|-------|---------|----------------------|
| Nœud **IN** de la salle `i` | `2*i` | 6 |
| Nœud **OUT** de la salle `i` | `2*i + 1` | 7 |
| Salle à partir d'un nœud `n` | `n / 2` | 6/2=3, 7/2=3 |

- **source** = `2*start_id + 1` (OUT de start)
- **puits**  = `2*end_id`       (IN de end)

---

## Mini-FAQ express

- **« 1 fourmi = ? »** → 1 unité de flot.
- **« 1 salle = ? »** → capacité 1 (sauf start/end), imposée par dédoublement.
- **« Pourquoi BFS ? »** → garantie polynomiale (Edmonds-Karp).
- **« Pourquoi arêtes inverses ? »** → reroutage = correction de Ford-Fulkerson.
- **« Flot max = ? »** → nombre max de chemins disjoints = fourmis avançant en parallèle.

Retour à l'[index](00_INDEX.md).
