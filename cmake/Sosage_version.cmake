set(SOSAGE_VERSION_MAJOR 1)
set(SOSAGE_VERSION_MINOR 2)
set(SOSAGE_VERSION_PATCH 1)
set(SOSAGE_VERSION_NAME "summer2022demo1")

add_definitions(-DSOSAGE_VERSION_MAJOR=${SOSAGE_VERSION_MAJOR})
add_definitions(-DSOSAGE_VERSION_MINOR=${SOSAGE_VERSION_MINOR})
add_definitions(-DSOSAGE_VERSION_PATCH=${SOSAGE_VERSION_PATCH})
add_definitions(-DSOSAGE_VERSION_NAME="${SOSAGE_VERSION_NAME}")
