cmake_minimum_required(VERSION 2.8)
project(unabto_raspberrypi)

option(using_piface
        "set true if using PiFace" OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/unabto_project.cmake)

if(using_piface)
  message("Making makefiles to raspberry pi with piface demo")
else()
  message("Making makefiles to raspberry pi demo")
endif()


set(src
  ${unabto_src}
  ${unabto_module_cli_gopt_src}  
  ${CMAKE_CURRENT_SOURCE_DIR}/src/unabto_main.c
)
if(using_piface)
  list(APPEND src ${CMAKE_CURRENT_SOURCE_DIR}/src/unabto_application_piface.c)
else()
  list(APPEND src ${CMAKE_CURRENT_SOURCE_DIR}/src/unabto_application.c)
endif()


include_directories(
  ${unabto_include_directories}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_definitions(${unabto_definitions})

add_executable(${PROJECT_NAME} ${src})

target_link_libraries(${PROJECT_NAME} ${unabto_link_libraries})

if(using_piface)
  target_link_libraries(${PROJECT_NAME} piface-1.0)
endif()

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
