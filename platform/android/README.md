# Sosage pour Android

## Compilation

1. Télécharger et installer SDK+NDK Android
2. `mkdir build && cd build`
3. `$ cmake .. -DCMAKE_BUILD_TYPE=Release -DSOSAGE_ANDROID:BOOL=ON`
4. Renseigner toutes variables cmake `*_SOURCE` (sources de SDL, Yaml
   et LZ4)
5. `cd android`
6. `./gradlew assembleDeub`
