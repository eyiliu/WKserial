LIST(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
FIND_PACKAGE(ROOT REQUIRED)
INCLUDE(${ROOT_USE_FILE})

GET_FILENAME_COMPONENT(DEV_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
SET(LIB_NAME WK${DEV_NAME})

INCLUDE_DIRECTORIES(.)
AUX_SOURCE_DIRECTORY(. OBJECT_SRC)

SET(LIB_DICT_NAME ROOT${LIB_NAME})
SET(LIB_DICT_CXX ${CMAKE_CURRENT_BINARY_DIR}/${LIB_DICT_NAME}.cxx)
SET(LIB_DICT_PCM ${CMAKE_CURRENT_BINARY_DIR}/lib${LIB_DICT_NAME}_rdict.pcm)
SET(LIB_DICT_MAP ${CMAKE_CURRENT_BINARY_DIR}/lib${LIB_DICT_NAME}.rootmap)

ROOT_GENERATE_DICTIONARY(${LIB_DICT_NAME} compatible.hh LINKDEF LinkDef.hh)
ADD_LIBRARY(${LIB_NAME} SHARED ${OBJECT_SRC} ${LIB_DICT_CXX})
TARGET_LINK_LIBRARIES(${LIB_NAME} ${PROJECT_NAME} ${ROOT_LIBRARIES})

install(
  TARGETS ${LIB_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

install(
  FILES ${LIB_DICT_PCM} ${LIB_DICT_MAP} 
  DESTINATION lib
  )
