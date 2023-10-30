set(KODA_IR_SRC IRBuilder.cpp ProgramGraph.cpp BasicBlock.cpp Instruction.cpp)

add_library(koda_IR STATIC ${KODA_IR_SRC})
add_library(koda::IR ALIAS koda_IR)

set (KODA_IR_PRINTER_SRC IRPrinter IRPrinter.cpp)

add_library(koda_IR_printer STATIC ${KODA_IR_PRINTER_SRC})
add_library(koda::IR::printer ALIAS koda_IR_printer)

target_link_libraries(koda_IR_printer koda::IR)