cmake_minimum_required (VERSION 3.10)
set (PostgreSQL_ADDITIONAL_VERSIONS "16")
project (ClientDB)
add_executable(ClientDB main.cpp)

#include libpqxx
add_subdirectory("C:/Program Files/libpqxx-7.8.1" libpqxx-build)
target_compile_features(ClientDB PRIVATE cxx_std_17)
target_link_libraries (ClientDB pqxx)