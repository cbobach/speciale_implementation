set(STORAGE_SRCS duplo-util/storage.cpp)
add_library(STORAGE ${STORAGE_SRCS})

set(CIRCUIT_SRCS circuit/circuit.cpp)
add_library(CIRCUIT ${CIRCUIT_SRCS})

set(GARBLING_SRCS garbling/garbling-handler.cpp garbling/garbled-circuit.cpp garbling/const-garbled-circuit.cpp garbling/eval-garbled-circuit.cpp)
add_library(GARBLING ${GARBLING_SRCS})
target_link_libraries(GARBLING BYTEARRAYVEC)

set(DUPLO_SRCS duplo/duplo-evaluator.cpp duplo/duplo-constructor.cpp duplo/duplo.cpp)
add_library(DUPLO ${DUPLO_SRCS})
target_link_libraries(DUPLO STORAGE GARBLING CIRCUIT SPLIT_COMMIT)

add_executable(DuploConstructor duplo-mains/duplo-const-main.cpp)
target_link_libraries(DuploConstructor DUPLO)

add_executable(DuploEvaluator duplo-mains/duplo-eval-main.cpp)
target_link_libraries(DuploEvaluator DUPLO)

add_executable(PokerConstructor duplo-mains/poker-const-main.cpp)
target_link_libraries(PokerConstructor DUPLO)

add_executable(PokerEvaluator duplo-mains/poker-eval-main.cpp)
target_link_libraries(PokerEvaluator DUPLO)

add_executable(PokerClient duplo-mains/poker-client-main.cpp)
target_link_libraries(PokerClient DUPLO)

add_executable(PokerPaser duplo-mains/poker-paser.cpp)
target_link_libraries(PokerPaser DUPLO gtest_main gtest)

#Only build TableProducer if GMP is found
if(${GMP_FOUND})

add_executable(TableProducer duplo-mains/table-producer.cpp)
target_link_libraries(TableProducer ${GMP_LIBRARIES} ${GMPXX_LIBRARIES} DUPLO)

else(${GMP_FOUND})

message("==================")
message("TableProducer not built due to GMP not found")
message("==================")

endif(${GMP_FOUND})