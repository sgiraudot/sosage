set(SOSAGE_VERSION_MAJOR 1)
set(SOSAGE_VERSION_MINOR 3)
set(SOSAGE_VERSION_PATCH 3)
set(SOSAGE_VERSION_NAME "superfluous3")
math(EXPR SOSAGE_VERSION_NUMBER "${SOSAGE_VERSION_MAJOR} * 10000 + ${SOSAGE_VERSION_MINOR} * 100 + ${SOSAGE_VERSION_PATCH}")

add_definitions(-DSOSAGE_VERSION_MAJOR=${SOSAGE_VERSION_MAJOR})
add_definitions(-DSOSAGE_VERSION_MINOR=${SOSAGE_VERSION_MINOR})
add_definitions(-DSOSAGE_VERSION_PATCH=${SOSAGE_VERSION_PATCH})
add_definitions(-DSOSAGE_VERSION_NAME="${SOSAGE_VERSION_NAME}")
