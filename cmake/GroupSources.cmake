# usage: GroupSources("${PROJECT_SOURCE_DIR}/inc/", "inc")
MACRO(GroupSources dir prefix)
    # file(GLOB_RECURSE elements RELATIVE ${dir} *.h *.hpp *.c *.cpp *.cc)
    FILE(GLOB_RECURSE elements RELATIVE ${dir} *.*)

    FOREACH(element ${elements})
      # Extract filename and directory
      GET_FILENAME_COMPONENT(element_name ${element} NAME)
      GET_FILENAME_COMPONENT(element_dir ${element} DIRECTORY)

      IF (NOT ${element_dir} STREQUAL "")
        # If the file is in a subdirectory use it as source group.
          # Use the full hierarchical structure to build source_groups.
          STRING(REPLACE "/" "\\" group_name ${element_dir})
          SOURCE_GROUP("${prefix}\\${group_name}" FILES ${dir}/${element})
      ELSE()
        # If the file is in the root directory, place it in the root source_group.
        SOURCE_GROUP("${prefix}\\" FILES ${dir}/${element})
      ENDIF()
    ENDFOREACH()
ENDMACRO()