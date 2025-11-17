cmake_minimum_required(VERSION 3.30.5)

project(metatron VERSION 0.0.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/prepare.cmake)
include(cmake/collect.cmake)
include(cmake/connect.cmake)
include(cmake/deploy.cmake)

prepare()
collect()
connect()
deploy()
