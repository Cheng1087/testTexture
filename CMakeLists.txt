cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
cmake_policy(SET CMP0074 NEW)

PROJECT(testCutTexture)
enable_testing()
#find_package(GTest REQUIRED)
include(config_3rd_libraries.cmake)
include(config_ec_libraries.cmake)
#include(config_library.cmake)
set(CMAKE_AUTOMOC TRUE)
set(CMAKE_AUTORCC TRUE)
set(srcTest
    DisplayBlock3DForCutsInOneNodeJoint.h
    DisplayBlock3DForCutsInOneNodeJoint.cpp
    DisplayCuttingResultWindow.h
    DisplayCuttingResultWindow.cpp
    vtkECPolyDataNormals.h
    vtkECPolyDataNormals.cpp
    vtkTextureMapToECObject.h
    vtkTextureMapToECObject.cpp
#    CuttingWidget.h
#    CuttingWidget.cpp
    BlockPickerInteractorStyle.h
    mainDisplayCuttingResultWindow.cpp
    model.qrc
    )
add_executable(testCutTexture
    ${srcTest}
    )

target_link_libraries(testCutTexture
    ${EC_extra_libs}
    ${ECComponents_LIBRARIES}
    ${ECCAD_LIBRARIES}
    ${ECIO_LIBRARIES}
    ${ECVisualization_LIBRARIES}
    ${ECCut_LIBRARIES}
    gtestd
    gtest_maind
    gmockd
    gmock_maind
    )


