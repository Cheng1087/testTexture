include(GenerateExportHeader)
set(EC_extra_libs "")

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/inst")

##################################
# ECComponents
##################################
if(WIN32)
    set(ECComponents_ROOT D:/lib/ECBase/0.1.8 )
endif()
set(CMAKE_MODULE_PATH "${ECComponents_ROOT}/cmake" ${CMAKE_MODULE_PATH})

message(STATUS CMAKE_MODULE_PATH" " ${CMAKE_MODULE_PATH})
foreach(var ${CMAKE_MODULE_PATH})
    message(STATUS ${var})
endforeach()
find_package(ECComponents REQUIRED)


message(STATUS "find libraries ")
foreach(var ${ECComponents_LIBRARIES})
    message(STATUS ${var})
endforeach()

include_directories(${ECComponents_INCLUDE_DIRS})
#message(STATUS "include dir : " ${ECComponents_INCLUDE_DIRS})

##################################
# ECCut
##################################
if(WIN32)
    set(ECCut_ROOT D:/lib/ECCut/0.2.2 )
endif()
set(CMAKE_MODULE_PATH "${ECCut_ROOT}/cmake" ${CMAKE_MODULE_PATH})

message(STATUS CMAKE_MODULE_PATH" " ${CMAKE_MODULE_PATH})
foreach(var ${CMAKE_MODULE_PATH})
    message(STATUS ${var})
endforeach()
find_package(ECCut REQUIRED)


message(STATUS "find libraries ")
foreach(var ${ECCut_LIBRARIES})
    message(STATUS ${var})
endforeach()

include_directories(${ECCut_INCLUDE_DIRS})
link_directories(${ECCut_LIBRARIES})
##################################
# ECVisualization
##################################
 if(WIN32)
     set(ECVisualization_ROOT "D:/lib/ECV"  )
 endif()
 set(CMAKE_MODULE_PATH "${ECVisualization_ROOT}/cmake" ${CMAKE_MODULE_PATH})

 message(STATUS CMAKE_MODULE_PATH" " ${CMAKE_MODULE_PATH})
 foreach(var ${CMAKE_MODULE_PATH})
     message(STATUS ${var})
 endforeach()
 find_package(ECVisualization REQUIRED)

 message(STATUS "find libraries ")
 foreach(var ${ECVisualization_LIBRARIES})
     message(STATUS ${var})
 endforeach()

 include_directories(${ECVisualization_INCLUDE_DIRS})
link_directories(${ECVisualization_LIBRARIES})
 ##################################
 # ECIO
 ##################################
  if(WIN32)
      set(ECIO_ROOT "D:/lib/ECIO"  )
  endif()
  set(CMAKE_MODULE_PATH "${ECIO_ROOT}/cmake" ${CMAKE_MODULE_PATH})

  message(STATUS CMAKE_MODULE_PATH" " ${CMAKE_MODULE_PATH})
  foreach(var ${CMAKE_MODULE_PATH})
      message(STATUS ${var})
  endforeach()
  find_package(ECIO REQUIRED)

  message(STATUS "find libraries ")
  foreach(var ${ECIO_LIBRARIES})
      message(STATUS ${var})
  endforeach()

  include_directories(${ECIO_INCLUDE_DIRS})
link_directories( ${ECIO_LIBRARIES}  )

##################################
# ECCAD
##################################
 if(WIN32)
     set(ECCAD_ROOT "D:/lib/ECCAD"  )
 endif()
 set(CMAKE_MODULE_PATH "${ECCAD_ROOT}/cmake" ${CMAKE_MODULE_PATH})

 message(STATUS CMAKE_MODULE_PATH" " ${CMAKE_MODULE_PATH})
 foreach(var ${CMAKE_MODULE_PATH})
     message(STATUS ${var})
 endforeach()
 find_package(ECCAD REQUIRED)

 message(STATUS "find libraries ")
 foreach(var ${ECCAD_LIBRARIES})
     message(STATUS ${var})
 endforeach()

 include_directories(${ECCAD_INCLUDE_DIRS})
link_directories( ${ECCAD_LIBRARIES}  )
