#
# cinf cmake module
#
# invoking cinfcc.py to reflect on source files for a target, merge the
# results and compiling into a library that can be linked with the target
# for runtime access to cinf reflection metadata.
#

macro(cinf_target_reflect target target_lib)

    # transform includes for this target
    get_target_property(${target}_includes ${target} INCLUDE_DIRECTORIES)
    list(TRANSFORM ${target}_includes PREPEND "-I")
    list(JOIN ${target}_includes " " ${target}_include_args)

    # add custom commands that reflect on sources for this target
    get_target_property(${target}_sources ${target} SOURCES)
    set(_source_inf "")
    foreach(_source ${${target}_sources})
        list(APPEND _source_inf ${_source}.cinf)
        add_custom_command(
            OUTPUT ${_source}.cinf
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cinfcc.py -p ${CMAKE_BINARY_DIR}
                -o ${CMAKE_BINARY_DIR}/${_source}.cinf ${${target}_include_args}
                   ${CMAKE_CURRENT_SOURCE_DIR}/${_source}
            DEPENDS ${_source} cinfcc VERBATIM)
    endforeach()
    list(TRANSFORM _source_inf PREPEND "${CMAKE_BINARY_DIR}/")

    # merge reflection metadata into an archive
    set(_archive_inf "${target}.cinf")
    add_custom_command(
        OUTPUT ${_archive_inf}
        COMMAND ${CMAKE_BINARY_DIR}/cinftool --merge
            ${CMAKE_BINARY_DIR}/${_archive_inf} ${_source_inf}
        DEPENDS ${_source_inf} cinftool VERBATIM)

    # create library containing reflection archive
    set(_archive_source "${target}.cinf.c")
    add_custom_command(
        OUTPUT ${_archive_source}
        COMMAND ${CMAKE_BINARY_DIR}/cinftool --emit
            ${CMAKE_BINARY_DIR}/${_archive_source} ${CMAKE_BINARY_DIR}/${_archive_inf}
        DEPENDS ${CMAKE_BINARY_DIR}/${_archive_inf} cinftool VERBATIM)
    add_library(${target_lib} ${_archive_source})

endmacro()
