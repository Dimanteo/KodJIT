set(KODA_CORE_SRC Compiler.cpp Analyses.cpp Passes.cpp)

add_library(koda_core STATIC ${KODA_CORE_SRC})
add_library(koda::core ALIAS koda_core)