# 7. Questions de soutenance — Q/R + défauts à assumer

Le fichier le plus important. Lis-le **deux fois**. Les réponses sont calibrées pour être
dites à l'oral.

---

## A. Questions « concept » (les plus probables)

**Q1. C'est quoi ton algo, en une phrase ?**
> On modélise la fourmilière en graphe et on résout un **flot maximum** avec **Edmonds-Karp**.
> Chaque unité de flot est une fourmi ; le flot max donne le nombre de chemins parallèles
> disjoints. On extrait ces chemins, on y répartit les fourmis de façon gloutonne, puis on
> simule tour par tour.

**Q2. Pourquoi un flot maximum et pas un plus court chemin ?**
> Parce qu'une salle ne tient qu'une fourmi : un chemin ne débite qu'une fourmi par tour.
> Avec beaucoup de fourmis, il faut **plusieurs chemins en parallèle**. Maximiser le nombre
> de chemins disjoints = maximiser le débit = un problème de flot maximum.

**Q3. Comment imposes-tu « une seule fourmi par salle » ?**
> Par le **dédoublement de nœuds**. Chaque salle `i` devient un nœud IN (`2i`) et un nœud OUT
> (`2i+1`) reliés par une arête interne de **capacité 1**. Tout chemin traversant la salle
> passe par cette arête, donc au plus une fourmi peut la traverser. (start/end ont une
> capacité interne `nb_ants`, mais elles sont hors du chemin source→puits donc sans effet.)

**Q4. Pourquoi BFS et pas DFS dans Ford-Fulkerson ?**
> Avec BFS, c'est **Edmonds-Karp** : on prend toujours le **plus court** chemin augmentant,
> ce qui borne le nombre d'augmentations à `O(V·E)` → complexité **polynomiale garantie**
> `O(V·E²)`, indépendante des capacités. Avec DFS, on n'a pas cette garantie.

**Q5. À quoi servent les arêtes inverses / le réseau résiduel ?**
> Ce sont les boutons « annuler ». Pousser du flot sur une arête inverse **défait** du flot
> placé sur la directe, ce qui permet de **rerouter** les chemins. Sans elles,
> Ford-Fulkerson pourrait rester bloqué sur un flot sous-optimal : elles sont nécessaires à
> la **correction** de l'algorithme. (cf. exemple [03 §3.5](03_EDMONDS_KARP.md).)

**Q6. Capacité résiduelle, c'est quoi ?**
> `capacity - flow` : ce qu'on peut encore pousser. L'arête directe part à `c`, l'inverse à
> `0` ; quand on pousse `f` sur la directe, l'inverse récupère `f` de résiduelle
> (`0 - (-f) = f`).

**Q7. Comment passes-tu du flot aux chemins concrets ?**
> Je pars de start et je **suis les arêtes qui portent du flot** (`flow > 0`). Comme chaque
> salle interne est de capacité 1, il n'y a qu'une arête sortante avec du flot → je remonte
> sans ambiguïté jusqu'à end. `nœud / 2` redonne l'id de la salle, `nœud + 1` passe du IN au
> OUT. (cf. [04](04_EXTRACTION_CHEMINS.md).)

**Q8. Comment répartis-tu les fourmis sur les chemins ?**
> Gloutonnement : pour chaque fourmi, je choisis le chemin de **score minimal**
> `(longueur-1) + fourmis_déjà_dessus`, qui estime son tour d'arrivée. Ça équilibre la charge
> selon les longueurs et minimise le tour d'arrivée de la dernière fourmi. (cf. [05 §5.2](05_SIMULATION.md).)

**Q9. Complexité ?**
> Edmonds-Karp : `O(V·E²)` en général ; ici, capacités à 1 et flot `F ≤ degré(start)`, donc
> en pratique `O(F·(V+E))`. Mémoire `O(V+E)` grâce aux listes d'adjacence. (cf. [06](06_COMPLEXITE_ET_PREUVES.md).)

**Q10. Théorème qui justifie la correction ?**
> **Flot-max / coupe-min** : flot max = capacité de la coupe min. Combiné au dédoublement,
> c'est le **théorème de Menger** version sommets → flot max = nombre max de chemins
> disjoints en sommets. La capacité 1 garantit leur disjonction donc l'absence de collision.

---

## B. Questions « code » (montre que tu sais lire le code)

**Q11. Pourquoi listes d'adjacence et pas matrice ?**
> Graphe creux + jusqu'à plusieurs milliers de salles. Une matrice `(2N)²` exploserait la
> mémoire ; la liste d'adjacence est `O(V+E)` et ne parcourt que les arêtes réelles. (Le
> brouillon `ALGORITHME.md` parlait de matrices, mais l'implémentation finale a tranché pour
> les listes — c'est un document de conception périmé.)

**Q12. Que fait `rev_edge_idx` ?**
> Depuis une arête, il donne l'**index de son arête inverse** chez le nœud destination, pour
> mettre à jour `flow` des deux côtés en `O(1)` lors de l'augmentation.

**Q13. Que valent source et puits exactement ?**
> `source = 2*start_id + 1` (le OUT de start), `sink = 2*end_id` (le IN de end).

**Q14. Comment gères-tu le cas « pas de chemin » ?**
> `compute_max_flow` renvoie le flot ; si `max_flow <= 0`, [main.c](../srcs/main.c) affiche
> `ERROR`. C'est la map `09_no_path.map`.

**Q15. `path_flow = 1000000`, c'est quoi ce nombre magique ?**
> C'est un `+∞` pratique pour initialiser la recherche du goulot (le minimum des résiduelles).
> Toutes les capacités étant petites, n'importe quelle borne supérieure large convient.
> *(Idéalement on utiliserait `INT_MAX`.)*

---

## C. ⚠️ Défauts connus — à ASSUMER, pas à cacher

Les examinateurs respectent l'honnêteté lucide. Connais ces 4 points :

**D1. `printf("DEBUG: …")` dans [main.c:14](../srcs/main.c).**
> *« Il reste une ligne de debug qui pollue la sortie standard ; en production elle doit être
> retirée. C'est un oubli de nettoyage, pas un problème d'algo. »*
> **Correctif** : supprimer la ligne. (Une démo propre = la retirer **avant** la soutenance.)

**D2. Espace parasite en début de certaines lignes de mouvements.**
> *« La séparation des mouvements utilise `if (i > 1) printf(" ")` ; si la fourmi 1 ne bouge
> pas ce tour-là, on obtient un espace en tête de ligne. C'est cosmétique, la logique reste
> correcte. »*
> **Correctif** : utiliser un drapeau « premier mouvement imprimé du tour » plutôt que
> l'indice `i`.

**D3. `find_paths` (flow.c:344) est une fonction morte (`return 0`).**
> *« C'est un vestige de l'API initiale ; le travail réel est fait par `extract_paths`. À
> supprimer pour la propreté. »*

**D4. Optimalité du nombre de tours : cas adverses théoriques.**
> *« On calcule le flot maximum puis on répartit gloutonnement selon les longueurs. Dans de
> rares cartes pathologiques, l'optimum exigerait de tester chaque sous-ensemble de k
> chemins ; on ne le fait pas explicitement, mais la répartition gloutonne — qui n'envoie de
> fourmis sur un chemin long que si c'est rentable — donne l'optimum sur la quasi-totalité
> des cas. »* (cf. [06 §6.4](06_COMPLEXITE_ET_PREUVES.md).)

**D5. (mineur) `simulate_ants` est en `O(N²)` par tour.**
> *« Pour vérifier qu'une salle est libre, on scanne toutes les fourmis. On pourrait passer à
> O(N) avec un tableau “salle occupée ? ”. Acceptable aux tailles d'entrée du sujet. »*

---

## D. Si on te demande de modifier le code en live

**« Enlève la ligne DEBUG »** → supprimer `main.c:14`.

**« Affiche aussi le nombre de chemins trouvés »** → après `compute_max_flow`,
`printf("%d paths\n", lemin.nb_paths);` (sur `stderr` pour ne pas polluer la sortie).

**« Comment tu testerais une nouvelle map ? »** → `./lem-in < maps/valid/ma_map.map` ; il y a
aussi `run_tests.sh` qui passe les maps de `maps/valid` et `maps/invalid`.

**« Et si une salle pouvait contenir 2 fourmis ? »** → mettre la **capacité de l'arête
interne** de dédoublement à 2 au lieu de 1 (flow.c:103). Élégant : seul ce nombre change.

---

## E. Phrases de secours (si tu bloques)

- *« Le principe directeur : 1 fourmi = 1 unité de flot, 1 salle = capacité 1 via
  dédoublement de nœud. »*
- *« BFS pour la garantie polynomiale, arêtes inverses pour la correction, flot-max/coupe-min
  pour la maximalité. »*
- *« Le flot donne les chemins ; le glouton équilibre les fourmis selon les longueurs ; la
  simulation respecte une fourmi par salle. »*

➡️ Dernier fichier : [08_GLOSSAIRE.md](08_GLOSSAIRE.md).
