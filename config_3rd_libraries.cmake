
set(EC_extra_libs "")
############################
# for assimp
###########################

if(WIN32)
    set(ASSIMP_ROOT_DIR "D:/lib/Assimp")

    set(CMAKE_PREFIX_PATH
        ${ASSIMP_ROOT_DIR}
        ${CMAKE_PREFIX_PATH}
    )

    set(CMAKE_MODULE_PATH
        "${ASSIMP_ROOT_DIR}/lib/cmake/assimp-4.1"
        ${CMAKE_MODULE_PATH})

    set(ASSIMP_INCLUDE_DIRS "${ASSIMP_ROOT_DIR}/include")
    set(ASSIMP_LIBRARIES "${ASSIMP_ROOT_DIR}/lib")
endif()
find_package(assimp REQUIRED)
message(STATUS "assimp libs : " ${ASSIMP_LIBRARIES})
message("assimp include dir: " ${ASSIMP_INCLUDE_DIRS})

include_directories(${ASSIMP_INCLUDE_DIRS})
link_directories(${ASSIMP_LIBRARY_DIRS})
list(APPEND EC_extra_libs ${ASSIMP_LIBRARIES} )

####################################
#  find Boost
####################################
# set(Boost_DEBUG ON) # set ON to see debug minfo when finding boost

# !!! very important definition !!!
# that will avoid undefined references in linking
add_definitions(-DBOOST_ALL_DYN_LINK)
# !!! very important definition !!!
# that will avoid undefined references in linking
#add_definitions(-DBOOST_LOG_DYN_LINK)

set(BOOST_ROOT "D:/lib/Boost")

set(Boost_USE_DEBUG_LIBS ON)
set(Boost_USE_RELEASE_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.68.0 REQUIRED COMPONENTS filesystem thread log log_setup)
#message(STATUS "found libs : " ${Boost_LIBRARIES})
foreach( var in ${Boost_LIBRARIES})
    message(${var} )
endforeach()
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
list(APPEND EC_extra_libs ${Boost_LIBRARIES})


####################################
#  find OpenMP
####################################
#FIND_PACKAGE( OpenMP REQUIRED)
#if(OPENMP_FOUND)
#    message("OPENMP FOUND")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
#endif()

####################################
#  find Eigen3
####################################
if(WIN32)
    set(Eigen3_DIR "D:/lib/eigen3")
    message(STATUS  "eigen3 cmake module path : " ${CMAKE_MODULE_PATH})
endif(WIN32)

set(CMAKE_PREFIX_PATH ${Eigen3_DIR} ${CMAKE_PREFIX_PATH})
set(CMAKE_MODULE_PATH "${Eigen3_DIR}/share" ${CMAKE_MODULE_PATH})

find_package (Eigen3 REQUIRED NO_MODULE)
include_directories(
  ${EIGEN3_INCLUDE_DIR}
  ${EIGEN3_LIBRARIES}
)

####################################
#  find Qt5
####################################
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0071 NEW)

if(WIN32)
    set(CMAKE_PREFIX_PATH
        "C:/Qt/Qt5.10.0/5.12.3/msvc2017_64"
        ${CMAKE_PREFIX_PATH}
        )
# elseif(UNIX)
endif(WIN32)

find_package(Qt5Core )

####################################
#  find Xerces
####################################
#find_package(XercesC)
#message(STATUS "xerces libs:" ${XercesC_LIBRARIES})
#include_directories(${XercesC_INCLUDE_DIRS})
#list(APPEND EC_extra_libs ${XercesC_LIBRARIES})

###################################
# rapidjson
###################################
if(WIN32)
    set(RapidJson_DIR "D:/lib/rapidjson/include")
endif(WIN32)
#find_package(RapidJSON)
include_directories(${RapidJson_DIR})

#####################################
# VTK
#####################################
if(WIN32)
    list(APPEND CMAKE_PREFIX_PATH
        C:/Program Files/VTK/lib/cmake/vtk-8.1)

    set(VTK_DIR C:/Program Files/VTK/bin)
endif()
find_package(VTK REQUIRED)
#include(${VTK_USE_FILE})
INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
link_libraries(${VTK_LIBRARIES})
list(APPEND EC_extra_libs ${VTK_LIBRARIES})
#####################################
# google test
#####################################
if(WIN32)
    # for windows find_package(GTest REQUIRED) DONOT work
    set(GTEST_ROOT "D:/lib/googletest/googletest-release-1.8.1-inst")
    set(GTEST_MSVC_SEARCH MT)
else()
    set(GTEST_ROOT /usr/local)
    # find_package(GTest REQUIRED)
endif()
#find_package(GTest REQUIRED)
include(GoogleTest)
include_directories(${GTEST_ROOT}/include)
link_directories(${GTEST_ROOT}/lib)
list(APPEND EC_extra_libs ${GTEST_LIBRARIES})

#################################
# find all needed packages
#################################

message(STATUS "added extra libs:")
foreach( var ${EC_extra_libs})
    message(${var} )
endforeach()
