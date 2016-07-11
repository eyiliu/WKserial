cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(wkserial)

if(CMAKE_COMPILER_IS_GNUCC)
   list(APPEND CMAKE_CXX_FLAGS "-fPIC" )
   list(APPEND CMAKE_LD_FLAGS "-fPIC" )
endif()


set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR} CACHE PATH "Prefix of install directories" FORCE)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include(${PROJECT_SOURCE_DIR}/cmake/FindROOT.cmake)



# set(ROOTCINT_EXECUTABLE root)
# #----------------------------------------------------------------------------
# # function ROOT_GENERATE_DICTIONARY( dictionary
# #                                    header1 header2 ...
# #                                    LINKDEF linkdef1 ...
# #                                    OPTIONS opt1...)
# function(ROOT_GENERATE_DICTIONARY dictionary)
#   CMAKE_PARSE_ARGUMENTS(ARG "" "" "LINKDEF;OPTIONS" "" ${ARGN})
#   #---Get the list of include directories------------------
#   get_directory_property(incdirs INCLUDE_DIRECTORIES)
#   set(includedirs)
#   foreach( d ${incdirs})
#      set(includedirs ${includedirs} -I${d})
#   endforeach()
#   #---Get the list of header files-------------------------
#   set(headerfiles)
#   foreach(fp ${ARG_UNPARSED_ARGUMENTS})
#     if(${fp} MATCHES "[*?]") # Is this header a globbing expression?
#       file(GLOB files ${fp})
#       foreach(f ${files})
#         if(NOT f MATCHES LinkDef) # skip LinkDefs from globbing result
#           set(headerfiles ${headerfiles} ${f})
#         endif()
#       endforeach()
#     else()
#       find_file(headerFile ${fp} HINTS ${incdirs})
#       set(headerfiles ${headerfiles} ${headerFile})
#       unset(headerFile CACHE)
#     endif()
#   endforeach()
#   #---Get LinkDef.h file------------------------------------
#   set(linkdefs)
#   foreach( f ${ARG_LINKDEF})
#     find_file(linkFile ${f} HINTS ${incdirs})
#     set(linkdefs ${linkdefs} ${linkFile})
#     unset(linkFile CACHE)
#   endforeach()
#   #---call rootcint------------------------------------------
#   add_custom_command(OUTPUT ${dictionary}.cxx
#                      COMMAND ${ROOTCINT_EXECUTABLE} -cint -f  ${dictionary}.cxx
#                                           -c ${ARG_OPTIONS} ${includedirs} ${headerfiles} ${linkdefs}
#                      DEPENDS ${headerfiles} ${linkdefs} VERBATIM)
# endfunction()
# #--------------------------------------------------------




add_subdirectory(source)

