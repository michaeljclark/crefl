#
# crefl cmake module
#
# invoking creflcc.py to reflect on source files for a target, merge the
# results and compiling into a library that can be linked with the target
# for runtime access to crefl reflection metadata.
#

macro(crefl_target_reflect target target_lib)

	# transform includes for this target
    get_target_property(${target}_includes ${target} INCLUDE_DIRECTORIES)
    list(TRANSFORM ${target}_includes PREPEND "-I")
    list(JOIN ${target}_includes " " ${target}_include_args)

    # add custom commands that reflect on sources for this target
    get_target_property(${target}_sources ${target} SOURCES)
    set(_source_refl "")
    foreach(_source ${${target}_sources})
        list(APPEND _source_refl ${_source}.refl)
        add_custom_command(
            OUTPUT ${_source}.refl
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/creflcc.py -p ${CMAKE_BINARY_DIR}
                -o ${CMAKE_BINARY_DIR}/${_source}.refl ${${target}_include_args}
                   ${CMAKE_CURRENT_SOURCE_DIR}/${_source}
            DEPENDS ${_source} crefl VERBATIM)
	endforeach()
    list(TRANSFORM _source_refl PREPEND "${CMAKE_BINARY_DIR}/")

    # merge reflection metadata into an archive
    set(_archive_refl "${target}.refl")
    add_custom_command(
        OUTPUT ${_archive_refl}
        COMMAND ${CMAKE_BINARY_DIR}/crefltool --merge
            ${CMAKE_BINARY_DIR}/${_archive_refl} ${_source_refl}
        DEPENDS ${_source_refl} crefltool VERBATIM)

    # create library containing reflection archive
    set(_archive_source "${target}.refl.c")
    add_custom_command(
        OUTPUT ${_archive_source}
        COMMAND ${CMAKE_BINARY_DIR}/crefltool --emit
            ${CMAKE_BINARY_DIR}/${_archive_source} ${CMAKE_BINARY_DIR}/${_archive_refl}
        DEPENDS ${CMAKE_BINARY_DIR}/${_archive_refl} crefltool VERBATIM)
    add_library(${target_lib} ${_archive_source})

endmacro()
