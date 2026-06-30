# 6. Complexité, terminaison & preuves de correction

Section « théorie » : ce qui transforme une bonne soutenance en excellente soutenance.
Tu n'as pas besoin de tout réciter, mais d'être à l'aise si on creuse.

Notations : `V` = nombre de nœuds (`= 2 × nb_rooms`), `E` = nombre d'arêtes
(`≈ 2 × nb_links + nb_rooms`).

---

## 6.1 Complexité d'Edmonds-Karp

- **Un BFS** coûte `O(V + E)` (avec listes d'adjacence).
- **Nombre d'augmentations** : le théorème d'Edmonds-Karp garantit `O(V × E)` augmentations
  **indépendamment des capacités** (c'est tout l'intérêt du BFS : le plus court chemin
  augmentant ne fait que croître, ce qui borne le nombre d'itérations).
- **Total : `O(V × E²)`.**

> Remarque spécifique à lem-in : presque toutes les capacités valent **1**, donc chaque
> augmentation pousse 1 unité et le **flot maximum F ≤ degré(start)**. Le nombre
> d'augmentations est alors borné par `F`, et le coût pratique est plutôt
> **`O(F × (V + E))`** — très rapide sur des fourmilières réelles.

- **Mémoire** : `O(V + E)` grâce aux listes d'adjacence (pas de matrice `V²` — cf.
  [02_MODELISATION_GRAPHE.md](02_MODELISATION_GRAPHE.md) §2.4).

- **Simulation** ([simulate.c](../srcs/algorithm/simulate.c)) : à chaque tour, pour chaque
  fourmi on scanne les autres fourmis (`O(N²)` par tour). Honnête mais pas optimal — voir
  [07](07_QUESTIONS_SOUTENANCE.md) pour la piste d'amélioration (`O(N)` par tour avec un
  tableau d'occupation des salles).

---

## 6.2 Terminaison

- **Edmonds-Karp se termine** : toutes les capacités sont des entiers ; chaque augmentation
  ajoute au moins 1 au flot ; le flot est borné supérieurement par `degré(start) ≤ V`. Donc
  un nombre fini d'augmentations.
- **La simulation se termine** : à chaque tour, au moins une fourmi avance (le pipeline n'est
  jamais bloqué tant que des chemins existent), donc `ants_finished` croît et finit par
  atteindre `nb_ants`.

---

## 6.3 Correction : pourquoi le flot maximum donne les bons chemins

### Théorème flot-max / coupe-min (à citer)

> Dans un réseau, la **valeur du flot maximum** est égale à la **capacité de la coupe
> minimale** (la plus petite « barrière » d'arêtes qui sépare la source du puits).

Conséquence pour nous : le flot maximum = le **plus grand nombre de chemins disjoints en
sommets** de start à end (théorème de Menger, version sommets, obtenue justement grâce au
dédoublement de nœuds qui transforme la disjonction en sommets en disjonction en arêtes).

- **Disjonction garantie** : capacité interne 1 ⇒ chaque salle intermédiaire est sur **au
  plus un** chemin ⇒ les chemins extraits ne partagent **aucune** salle intermédiaire ⇒
  aucune collision possible pendant la simulation.
- **Maximalité garantie** : Ford-Fulkerson s'arrête quand il n'existe **plus** de chemin
  augmentant ; le théorème flot-max/coupe-min assure qu'à ce moment le flot est **maximal**.
  Les arêtes inverses sont ce qui rend cette maximalité atteignable (reroutage).

---

## 6.4 Le point d'honnêteté : flot maximum ≠ forcément tours minimum

C'est **la** nuance fine que les examinateurs aiment. À connaître :

- Notre code calcule le flot **maximum** (le plus de chemins disjoints possibles), puis
  répartit les fourmis dessus.
- Or **utiliser le plus de chemins n'est pas toujours optimal en tours** : un chemin
  supplémentaire mais très long peut faire perdre des tours au lieu d'en gagner.
- L'approche théoriquement complète serait : pour chaque `k` de 1 au flot max, calculer le
  **meilleur ensemble de `k` chemins** et garder le `k` qui minimise le nombre de tours
  (formule `max sur les chemins de (longueur + fourmis_assignées)`).

**Ce que fait notre implémentation :** elle prend l'ensemble du flot max, puis la
**répartition gloutonne** [simulate.c §5.2](05_SIMULATION.md) tient compte des longueurs et
**n'envoie de fourmis sur un chemin long que si c'est rentable** (un chemin trop long reçoit
0 fourmi car son score reste toujours supérieur). En pratique, pour la grande majorité des
fourmilières, ça donne le **nombre de tours optimal**.

> Formulation honnête en soutenance : *« On calcule le flot maximum puis on répartit
> gloutonnement en fonction des longueurs ; un chemin non rentable reçoit zéro fourmi. C'est
> optimal sur les cas usuels. Le cas théorique adverse — où il faudrait tester chaque
> sous-ensemble de k chemins — n'est pas géré explicitement, mais la répartition gloutonne
> l'absorbe dans la quasi-totalité des cartes. »*

---

## 6.5 Tableau récapitulatif (à mémoriser)

| Élément | Complexité | Garantie |
|---------|-----------|----------|
| BFS (1 chemin augmentant) | `O(V + E)` | trouve le + court chemin résiduel |
| Edmonds-Karp (flot max) | `O(V·E²)`, ici `O(F·(V+E))` | flot **maximal** (flot-max/coupe-min) |
| Extraction des chemins | `O(V + E)` | chemins **disjoints** en sommets |
| Répartition gloutonne | `O(N · nb_paths)` | minimise le tour d'arrivée de chaque fourmi |
| Simulation | `O(tours · N²)` | respecte « 1 fourmi / salle » |

➡️ Suite : [07_QUESTIONS_SOUTENANCE.md](07_QUESTIONS_SOUTENANCE.md) — questions/réponses et
défauts à assumer.
