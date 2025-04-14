set(core-headers ${path}/include/metatron/core)
target_precompile_headers(${target} ${access} ${core-headers}/prelude/prelude.hpp)
list(APPEND metatron-deps mimalloc rgb2spec tinyexr stb openimageio)
