message("processing sources ...")

file(GLOB srcs RELATIVE ${TSSSNI_SRC_DIR} ${TSSSNI_SRC_DIR}/*)
set(files "")
set(targets "")

foreach (src ${srcs})
  if (IS_DIRECTORY "${TSSSNI_SRC_DIR}/${src}")
    set(src-target tsssni.src.${src})
    add_library(${src-target} INTERFACE)

    set(setup ${TSSSNI_SRC_DIR}/${src}/setup.cmake)

    if (EXISTS ${setup})
      include(${setup})
    endif()

    get_property(link-libs TARGET ${src-target} PROPERTY tsssni-link-libs)
    if (link-libs)
      foreach (lib ${link-libs})
        target_link_libraries(${src-target} INTERFACE tsssni.lib.${lib})
        message("${src-target} link lib ${lib}")
      endforeach()
    endif()

    target_include_directories(${src-target} SYSTEM INTERFACE ${TSSNI_SRC_DIR}/${src})
  endif()
endforeach()

foreach (src ${srcs})
  if (IS_DIRECTORY "${TSSSNI_SRC_DIR}/${src}")
    set(src-target tsssni.src.${src})

    get_property(link-srcs TARGET ${src-target} PROPERTY tsssni-link-srcs)
    if (link-srcs)
      foreach (src ${link-srcs})
        target_link_libraries(${src-target} INTERFACE tsssni.src.${src})
        message("${src-target} link src ${src}")
      endforeach()
    endif()

    file(GLOB src-files ${TSSSNI_SRC_DIR}/${src}/*.cpp)
    message(...${src-files})
    list(APPEND files ${src-files})
    list(APPEND targets ${src-target})
  endif()
endforeach()

add_executable(tsssni-engine ${files})
target_link_libraries(tsssni-engine ${targets})
