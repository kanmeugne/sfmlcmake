# CMakeList.txt : Upper level configuration file (here you can add subprojects and/or manage external depenencies)
# et incluez les sous-projets ici.
#
cmake_minimum_required (VERSION 3.10)
# set global output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/${CONFIG})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}/${CONFIG})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/${CONFIG})

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/${CONFIG} )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}/${CONFIG} )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/${CONFIG} )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

project (HelloWorld C CXX)

# Incluez les sous-projets
add_subdirectory ("App")
