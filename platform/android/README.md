# Sosage for Android

## Compilation

1. Download and install the Android SDK and NDK
2. `mkdir build && cd build`
3. `$ cmake .. -DCMAKE_BUILD_TYPE=Release -DSOSAGE_ANDROID:BOOL=ON`
4. Fill all `*_SOURCE` cmake variables (SDL, Yaml and LZ4 sources)
5. `cd android`
6. `./gradlew assembleDebug`
