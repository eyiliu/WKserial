add_subdirectory(core)
add_subdirectory(device)

ADD_LIBRARY(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:core>) 


install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )
