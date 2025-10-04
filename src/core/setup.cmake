set(core-headers ${path}/include/metatron/core)
target_compile_options(${target} ${access}
    -include ${core-headers}/prelude/prelude.hpp
)
list(APPEND metatron-deps mimalloc proxy)
