cmake_minimum_required(VERSION 4.0.0)
project(metatron VERSION 0.2.0)

include(cmake/prepare.cmake)
include(cmake/collect.cmake)
include(cmake/connect.cmake)
include(cmake/deploy.cmake)

prepare()
collect()
connect()
deploy()
