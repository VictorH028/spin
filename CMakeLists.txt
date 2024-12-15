cmake_minimum_required(VERSION 3.10)


project(_spix)

set(CMAKE_CXX_STANDARD 17) 

add_executable(spix spix.cpp)


#########    Instalación  ############
install(TARGETS spix
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    FILES  ./man/spix.1 DESTINATION usr/share/man/man1
)
