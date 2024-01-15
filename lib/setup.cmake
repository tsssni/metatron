message("processing libraries ...")

file(GLOB libs RELATIVE ${TSSSNI_LIB_DIR} ${TSSSNI_LIB_DIR}/*)

foreach (lib ${libs})
  if (IS_DIRECTORY "${TSSSNI_LIB_DIR}/${lib}")
    set(lib-target tsssni.lib.${lib})
    add_library(${lib-target} INTERFACE)
    set_property(TARGET ${lib-target} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${TSSSNI_LIB_BUILD_DIR})

    set(setup ${TSSSNI_LIB_DIR}/${lib}/setup.cmake)

    if (EXISTS ${setup})
      include(${setup})
    endif()

    get_property(found-libs TARGET ${lib-target} PROPERTY tsssni-found-libs)
    if (found-libs)
      target_link_libraries(${lib-target} INTERFACE ${found-libs})
      message("${lib-target} link lib ${found-libs}")
    endif()
  endif()
endforeach()
