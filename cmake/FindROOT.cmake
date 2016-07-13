MESSAGE(STATUS "Looking for Root...")

find_program(ROOT_CINT_EXECUTABLE rootcint HINTS $ENV{ROOTSYS}/bin)
find_program(ROOT_CONFIG_EXECUTABLE root-config HINTS $ENV{ROOTSYS}/bin)
find_program(ROOT_GENREFLEX_EXECUTABLE genreflex HINTS $ENV{ROOTSYS}/bin)


SET(ROOT_DEFINITIONS "")


# FIND_PROGRAM(ROOT_CONFIG_EXECUTABLE NAMES root-config PATHS
#   ${ROOT_CONFIG_SEARCHPATH}
#   NO_DEFAULT_PATH)

IF (${ROOT_CONFIG_EXECUTABLE} MATCHES "ROOT_CONFIG_EXECUTABLE-NOTFOUND")
  IF (ROOT_FIND_REQUIRED)
    MESSAGE( FATAL_ERROR "ROOT not installed in the searchpath and ROOTSYS is not set. Please
 set ROOTSYS or add the path to your ROOT installation in the Macro FindROOT.cmake in the
 subdirectory cmake/modules.")
  ELSE(ROOT_FIND_REQUIRED)
    MESSAGE( STATUS "Could not find ROOT.")
  ENDIF(ROOT_FIND_REQUIRED)
ELSE (${ROOT_CONFIG_EXECUTABLE} MATCHES "ROOT_CONFIG_EXECUTABLE-NOTFOUND")
  STRING(REGEX REPLACE "(^.*)/bin/root-config" "\\1" test ${ROOT_CONFIG_EXECUTABLE}) 
  SET( ENV{ROOTSYS} ${test})
  set( ROOTSYS ${test})
ENDIF (${ROOT_CONFIG_EXECUTABLE} MATCHES "ROOT_CONFIG_EXECUTABLE-NOTFOUND")  

IF (WIN32)
  SET(ROOT_FOUND FALSE)
  IF (ROOT_CONFIG_EXECUTABLE)
    SET(ROOT_FOUND TRUE)
    set(ROOT_INCLUDE_DIR ${ROOTSYS}/include)
    set(ROOT_LIBRARY_DIR ${ROOTSYS}/lib)
    SET(ROOT_BINARY_DIR ${ROOTSYS}/bin)
    set(ROOT_LIBRARIES -LIBPATH:${ROOT_LIBRARY_DIR} libGpad.lib libHist.lib libGraf.lib libGraf3d.lib libTree.lib libRint.lib libPostscript.lib libMathCore.lib libRIO.lib libNet.lib libThread.lib libCore.lib libCint.lib libGui.lib libGuiBld.lib)
    FIND_PROGRAM(ROOT_CINT_EXECUTABLE
      NAMES rootcint
      PATHS ${ROOT_BINARY_DIR}
      NO_DEFAULT_PATH
      )
    MESSAGE(STATUS "Found ROOT: $ENV{ROOTSYS}/bin/root (WIN32/version not identified)")
  ENDIF (ROOT_CONFIG_EXECUTABLE)
  
ELSE(WIN32)
  IF (ROOT_CONFIG_EXECUTABLE)

    execute_process(
      COMMAND ${ROOT_CONFIG_EXECUTABLE} --prefix
      OUTPUT_VARIABLE ROOTSYS
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(
      COMMAND ${ROOT_CONFIG_EXECUTABLE} --version
      OUTPUT_VARIABLE ROOT_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(
      COMMAND ${ROOT_CONFIG_EXECUTABLE} --incdir
      OUTPUT_VARIABLE ROOT_INCLUDE_DIR
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(ROOT_INCLUDE_DIRS ${ROOT_INCLUDE_DIR})

    execute_process(
      COMMAND ${ROOT_CONFIG_EXECUTABLE} --libdir
      OUTPUT_VARIABLE ROOT_LIBRARY_DIR
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(ROOT_LIBRARY_DIRS ${ROOT_LIBRARY_DIR})

    
    EXEC_PROGRAM(${ROOT_CONFIG_EXECUTABLE} ARGS "--version" OUTPUT_VARIABLE ROOTVERSION)
    MESSAGE(STATUS "Found ROOT: $ENV{ROOTSYS}/bin/root (found version ${ROOTVERSION})")

        
    # and now the version string given by qmake
    STRING(REGEX REPLACE "^([0-9]+)\\.[0-9][0-9]+\\/[0-9][0-9]+.*" "\\1" found_root_major_vers "${ROOTVERSION}")
    STRING(REGEX REPLACE "^[0-9]+\\.([0-9][0-9])+\\/[0-9][0-9]+.*" "\\1" found_root_minor_vers "${ROOTVERSION}")
    STRING(REGEX REPLACE "^[0-9]+\\.[0-9][0-9]+\\/([0-9][0-9]+).*" "\\1" found_root_patch_vers "${ROOTVERSION}")

    IF (found_root_major_vers EQUAL 6)
      add_definitions(-DEUDAQ_LIB_ROOT6)
    ENDIF (found_root_major_vers EQUAL 6)
    
    MATH(EXPR found_vers "${found_root_major_vers}*10000 + ${found_root_minor_vers}*100 + ${found_root_patch_vers}")
    
    SET(ROOT_FOUND TRUE)
  ENDIF (ROOT_CONFIG_EXECUTABLE)


  IF (ROOT_FOUND)
    EXEC_PROGRAM( ${ROOT_CONFIG_EXECUTABLE}
      ARGS "--libdir"
      OUTPUT_VARIABLE ROOT_LIBRARY_DIR)

    EXEC_PROGRAM(${ROOT_CONFIG_EXECUTABLE}
      ARGS "--bindir"
      OUTPUT_VARIABLE root_bins )
    SET(ROOT_BINARY_DIR ${root_bins})

    EXEC_PROGRAM( ${ROOT_CONFIG_EXECUTABLE}
      ARGS "--incdir" 
      OUTPUT_VARIABLE root_headers )
    SET(ROOT_INCLUDE_DIR ${root_headers})

    EXEC_PROGRAM( ${ROOT_CONFIG_EXECUTABLE}
      ARGS "--glibs" 
      OUTPUT_VARIABLE root_flags )

  ENDIF (ROOT_FOUND)
ENDIF(WIN32)


#----------------------------------------------------------------------------
# function ROOT_GENERATE_DICTIONARY( dictionary
#                                    header1 header2 ...
#                                    LINKDEF linkdef1 ...
#                                    OPTIONS opt1...)
function(ROOT_GENERATE_DICTIONARY dictionary)
  CMAKE_PARSE_ARGUMENTS(ARG "" "" "LINKDEF;OPTIONS" "" ${ARGN})
  #---Get the list of include directories------------------
  get_directory_property(incdirs INCLUDE_DIRECTORIES)
  set(includedirs)
  foreach( d ${incdirs})
    set(includedirs ${includedirs} -I${d})
  endforeach()
  #---Get the list of header files-------------------------
  set(headerfiles)
  foreach(fp ${ARG_UNPARSED_ARGUMENTS})
    if(${fp} MATCHES "[*?]") # Is this header a globbing expression?
      file(GLOB files ${fp})
      foreach(f ${files})
        if(NOT f MATCHES LinkDef) # skip LinkDefs from globbing result
          set(headerfiles ${headerfiles} ${f})
        endif()
      endforeach()
    else()
      find_file(headerFile ${fp} HINTS ${incdirs})
      set(headerfiles ${headerfiles} ${headerFile})
      unset(headerFile CACHE)
    endif()
  endforeach()
  #---Get LinkDef.h file------------------------------------
  set(linkdefs)
  foreach( f ${ARG_LINKDEF})
    find_file(linkFile ${f} HINTS ${incdirs})
    set(linkdefs ${linkdefs} ${linkFile})
    unset(linkFile CACHE)
  endforeach()
  #---call rootcint------------------------------------------
  add_custom_command(OUTPUT ${dictionary}.cxx
    COMMAND ${ROOT_CINT_EXECUTABLE} -cint -f  ${dictionary}.cxx
    -c ${ARG_OPTIONS} ${includedirs} ${headerfiles} ${linkdefs}
    DEPENDS ${headerfiles} ${linkdefs} VERBATIM)
endfunction()
