# Systèmes de gestion

Ce projet sépare désormais chaque grand système dans un composant dédié afin de faciliter l'évolutivité et les tests.

## Alimentation (`components/feeding`)
- Fonctions prévues : `save_feeding_data()`, `load_feeding_data()`.
- Implémentation actuelle : squelette retournant toujours `false`.

## Habitat (`components/habitat`)
- Fonctions prévues : `save_habitat_data()`, `load_habitat_data()`.
- Implémentation actuelle : squelette retournant toujours `false`.

## Reproduction (`components/reproduction`)
- Fonctions prévues : `save_reproduction_data()`, `load_reproduction_data()`.
- Implémentation actuelle : squelette retournant toujours `false`.

## Santé (`components/health`)
- Fonctions prévues : `save_health_data()`, `load_health_data()`.
- Implémentation actuelle : squelette retournant toujours `false`.

Ces interfaces serviront de point d'entrée aux futurs développements; aucune logique métier n'est encore implémentée.
