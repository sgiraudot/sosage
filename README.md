# Sosage (Superfluous Open Source Adventure Game Engine)

## Dépendances

 - SDL2 (>= 2.0.10)
 - SDL2_Image
 - SDL2_TTF
 - SDL2_Mixer
 - libyaml
 - liblz4

## Données de jeu

Ce dépôt ne contient que le code source du moteur Sosage : les données
de jeu sont fournies séparément. Pour compiler le jeu, il faut
renseigner le dossier contenant les données de jeu via la variable
cmake `SOSAGE_DATA_FOLDER`. Ce dossier doit contenir le fichier
`config.cmake` fourni.

## Compilation (GNU/Linux)

__ATTENTION : les données du jeu _Superflu Riteurnz_ ne sont
compatibles qu'avec la version 1. Vous êtes actuellement sur la
branche `dev` qui est la branche de développement, veuillez utilisez
la branche `v1.0.x` pour compiler _Superflu Riteurnz_.__

1. `# apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev libyaml-dev liblz4-1`
2. `$ mkdir build && cd build`
3. `$ cmake .. -DCMAKE_BUILD_TYPE=Release -DSOSAGE_DATA_FOLDER=[where_the_data_is]`
4. `$ make`

## Autres plateformes

La compilation devrait être similaire pour Windows et Mac OS (à part
la gestion des dépendances, bien entendu).

En ce qui concerne les plateformes nécessitant un traitement
spécifique (Android, Emscripten, etc.), voir les différents
`README.md` dans `platform`.
