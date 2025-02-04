cmake_minimum_required(VERSION 3.0)
project(EXAMPLE)
find_package (Threads)
find_library(wiringPi_LIB wiringPi)
add_definitions(-DMC802=0)
set (PATH_LIB_UNIRAIL Libs_Unirail/)
include_directories(${PATH_LIB_UNIRAIL})


# Cree des variables avec les fichiers a compiler
set(SRCS
${PATH_LIB_UNIRAIL}CAN/canLinux.c
${PATH_LIB_UNIRAIL}CAN/MESCAN1_Utilitaire.c
${PATH_LIB_UNIRAIL}CAN/MESCAN1_VarStatusTrain.c
${PATH_LIB_UNIRAIL}CAN/MESCAN1_VarTrain.c
avance_train.c
)

set(HEADERS
${PATH_LIB_UNIRAIL}CAN/canLinux.h
${PATH_LIB_UNIRAIL}CAN/MESCAN1_Utilitaire.h
${PATH_LIB_UNIRAIL}CAN/loco_Parametres.h
${PATH_LIB_UNIRAIL}CAN/MESCAN1_DefinitionVariable.h
${PATH_LIB_UNIRAIL}CAN/MESCAN1_ID&DLC_LOCO.h
${PATH_LIB_UNIRAIL}CAN/MESCAN1_VarStatusTrain.h
${PATH_LIB_UNIRAIL}CAN/MESCAN1_VarTrain.h
)
set (EXECSRC avance_train.c)
set (EXECUTABLE avance_train.exe)

set(SRCS ${SRCS} ${EXECSRC} )
add_executable(${EXECUTABLE} ${SRCS} ${HEADERS})
target_compile_options(${EXECUTABLE} PRIVATE -Wall)

#inclusion des libs (equivalent a -lwiringPi -lpthread -lm)
target_link_libraries( ${EXECUTABLE} pthread )
target_link_libraries( ${EXECUTABLE} wiringPi )
target_link_libraries( ${EXECUTABLE} m )
target_link_libraries( ${EXECUTABLE} rt )