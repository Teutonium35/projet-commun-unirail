cmake_minimum_required(VERSION 3.0)
project(ConvertisseurCanModbusProject C)

add_definitions(-DMC802=0)

find_package (Threads)

# Ajouter les fichiers source
set(SRCS
main.c
../CAN/canLinux.c
../CAN/MESCAN1_Utilitaire.c

)

add_executable(ConvApp ${SRCS})

# Ajouter les fichiers d'en-tête pour l'inclusion
target_include_directories(ConvApp PRIVATE ../CAN)

# Lier les bibliothèques
target_link_libraries(ConvApp PRIVATE 
    m                      # Librairie mathématique
    pthread                # Threads POSIX
)

# Ajouter les options de compilation spécifiques à libmodbus
target_compile_options(ConvApp PRIVATE)
