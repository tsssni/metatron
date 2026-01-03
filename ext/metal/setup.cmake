if(system MATCHES "darwin")
    extend(metal-cpp)
    list(APPEND deps apple::metal-cpp)
endif()
