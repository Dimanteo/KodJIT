set(KODA_IR_SRC IRBuilder.cpp ProgramGraph.cpp)

add_library(koda_IR STATIC ${KODA_IR_SRC})
add_library(koda::IR ALIAS koda_IR)
