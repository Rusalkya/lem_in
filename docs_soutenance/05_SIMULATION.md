# 5. Répartition des fourmis & simulation tour par tour

Fichier [`simulate.c`](../srcs/algorithm/simulate.c). On a maintenant `nb_paths` chemins
disjoints. Deux questions restent :
1. **Quelle fourmi sur quel chemin ?** (répartition gloutonne)
2. **Comment afficher les déplacements tour par tour ?** (simulation)

---

## 5.1 Pourquoi ne pas tout mettre sur le chemin le plus court ?

Parce qu'un chemin = un tuyau qui débite **1 fourmi/tour**. Si on a 2 chemins, on veut les
**remplir tous les deux** pour avancer 2 fourmis par tour. Mais les chemins n'ont pas la même
**longueur** : envoyer une fourmi sur un chemin long coûte plus de tours avant qu'elle
arrive. Il faut donc **équilibrer** : un chemin court peut absorber plus de fourmis qu'un
chemin long.

---

## 5.2 La répartition gloutonne (*Greedy*)

[simulate.c:44-66](../srcs/algorithm/simulate.c). Pour chaque fourmi (de 1 à N), on choisit le
chemin qui **minimise le score** :

```c
score(p) = (longueur_du_chemin_p - 1) + nb_fourmis_déjà_sur_p
```

```c
int best_path = 0;
int min_score = (paths[0].length - 1) + path_ants_count[0];
for (p = 1; p < nb_paths; p++) {
    int score = (paths[p].length - 1) + path_ants_count[p];
    if (score < min_score) { min_score = score; best_path = p; }
}
path_assignment[i] = best_path;
path_ants_count[best_path]++;        // une fourmi de plus sur ce chemin
```

### Comprendre le score

`score(p)` **estime le tour d'arrivée** de la fourmi qu'on s'apprête à poser sur `p` :

- `longueur - 1` = nombre d'arêtes à traverser = nombre de tours pour parcourir le chemin
  une fois vide ;
- `+ nb_fourmis_déjà_dessus` = chaque fourmi déjà placée la fait attendre un tour de plus
  (file indienne).

On pose donc chaque fourmi sur le chemin où elle **arriverait le plus tôt** → cela
**minimise le tour d'arrivée de la dernière fourmi**, c'est-à-dire le **nombre total de
tours**. C'est l'algorithme glouton **optimal** classique pour distribuer des fourmis sur des
chemins disjoints de longueurs fixées.

### Exemple chiffré (10 fourmis, 2 chemins)

Chemins : court `length=3` (score de base 2), long `length=4` (score de base 3).

| Fourmi | score court | score long | choisi |
|:------:|:-----------:|:----------:|:------:|
| 1 | 2+0=2 | 3+0=3 | **court** |
| 2 | 2+1=3 | 3+0=3 | **court** (égalité → 1er) |
| 3 | 2+2=4 | 3+0=3 | **long** |
| 4 | 2+2=4 | 3+1=4 | **court** |
| … | | | … |

→ Le court reçoit plus de fourmis que le long, exactement comme attendu. C'est ce qui produit
la sortie en **7 tours** observée pour cette map.

---

## 5.3 La simulation tour par tour

[simulate.c:69-119](../srcs/algorithm/simulate.c). Tant que toutes les fourmis ne sont pas
arrivées, à chaque tour on parcourt les fourmis et on tente de les avancer d'une salle.

```c
while (ants_finished < nb_ants) {
    current_turn++;
    for (i = 1; i <= nb_ants; i++) {
        if (ants[i].current_room == end) continue;     // déjà arrivée
        int next_room = paths[path_assignment[i]].rooms[ants[i].position_in_path + 1];

        // une salle ne peut accueillir qu'une fourmi : vérifier qu'elle est libre
        int can_move = 1;
        for (j = 1; j <= nb_ants; j++)
            if (i != j && ants[j].current_room == next_room) can_move = 0;

        // sauf start/end : capacité illimitée
        if (next_room == start || next_room == end) can_move = 1;

        if (can_move) {
            ants[i].current_room = next_room;
            ants[i].position_in_path++;
            if (next_room == end) ants_finished++;
            printf("L%d-%s", i, rooms[next_room].name);   // affichage du mouvement
        }
    }
    printf("\n");
}
```

Idées à expliquer :
- Chaque fourmi suit **son** chemin assigné, salle par salle
  (`position_in_path` est son curseur).
- Avant d'avancer, on vérifie que la **salle suivante est libre** (aucune autre fourmi
  dedans) → respecte « une fourmi par salle ». Exception : `start` et `end` n'ont pas cette
  limite.
- Tous les mouvements d'un même tour sont sur **une seule ligne**, format `Lx-salle`.
- Comme les chemins sont **disjoints** (garanti par le flot), il n'y a pas de conflit entre
  fourmis de chemins différents — seules les fourmis d'un **même** chemin se suivent en file.

---

## 5.4 Le nombre de tours

Le nombre de tours total n'est pas calculé par une formule fermée : il **émerge** de la
simulation (la boucle `while` s'arrête quand `ants_finished == nb_ants`).

Pour un **seul** chemin de longueur `L` (en arêtes) avec `N` fourmis, on retrouve la formule
classique : **`tours = L + (N - 1)`** (`L` tours pour la 1ʳᵉ fourmi, puis une de plus arrive
à chaque tour suivant). Avec plusieurs chemins, la répartition gloutonne du §5.2 minimise ce
total.

---

## 5.5 ⚠️ Deux défauts d'affichage à connaître (à assumer en soutenance)

Détaillés dans [07_QUESTIONS_SOUTENANCE.md](07_QUESTIONS_SOUTENANCE.md), mais les voici :

1. **Espace parasite en début de ligne.** Le code fait `if (i > 1) printf(" ")` pour séparer
   les mouvements. Mais si la fourmi 1 **ne bouge pas** ce tour-là (elle est arrivée ou
   bloquée), la première fourmi qui bouge a `i > 1` et imprime quand même un espace en tête
   de ligne. Visible sur `02_two_paths` : `␣L2-end L3-end …`. Inoffensif logiquement, mais
   c'est un défaut de format.

2. **Ligne `DEBUG:` dans [main.c:14](../srcs/main.c).** Un `printf("DEBUG: …")` traîne sur la
   **sortie standard**. Ça **pollue la sortie** attendue par l'énoncé. À retirer avant
   évaluation officielle.

➡️ Suite : [06_COMPLEXITE_ET_PREUVES.md](06_COMPLEXITE_ET_PREUVES.md) — complexité,
terminaison et le théorème flot-max/coupe-min.
