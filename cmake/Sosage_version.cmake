set(SOSAGE_VERSION_MAJOR 1)
set(SOSAGE_VERSION_MINOR 4)
set(SOSAGE_VERSION_PATCH 2)
set(SOSAGE_VERSION_NAME "returnz2")
math(EXPR SOSAGE_VERSION_NUMBER "${SOSAGE_VERSION_MAJOR} * 1000000 + ${SOSAGE_VERSION_MINOR} * 10000 + ${SOSAGE_VERSION_PATCH} * 100 + ${SOSAGE_DATA_VERSION}")

add_definitions(-DSOSAGE_VERSION_MAJOR=${SOSAGE_VERSION_MAJOR})
add_definitions(-DSOSAGE_VERSION_MINOR=${SOSAGE_VERSION_MINOR})
add_definitions(-DSOSAGE_VERSION_PATCH=${SOSAGE_VERSION_PATCH})
add_definitions(-DSOSAGE_VERSION_NAME="${SOSAGE_VERSION_NAME}")
