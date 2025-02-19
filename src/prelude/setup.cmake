set(prelude-header ${path}/include/metatron/prelude/prelude.hpp)
target_precompile_headers(${target} ${access} ${prelude-header})
list(APPEND metatron-deps mimalloc)
