PROJECT(steppermotorserver)
cmake_minimum_required(VERSION 2.8)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(${PROJECT_NAME}_MAJOR_VERSION 00)
set(${PROJECT_NAME}_MINOR_VERSION 01)
set(${PROJECT_NAME}_PATCH_VERSION 03)
include(cmake/set_version_numbers.cmake)

# Set names for output binary and library
set(BINARY_OUTPUT_NAME "${PROJECT_NAME}")

# Moderate version of the compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -Wextra -ansi -Wuninitialized -O3 -fomit-frame-pointer")

# add constants for HOST and USER
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSER=\\\"`whoami`\\\" -DHOST=\\\"`hostname`\\\"")

#use -DCMAKE_BUILD_TYPE=Debug in your cmake command to turn on the coverage option
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3 --coverage")

# linker flags
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-dynamic -Wl,--no-as-needed")


include(cmake/add_dependency.cmake)
add_dependency(ChimeraTK-ApplicationCore 01.00 REQUIRED)
add_dependency(ChimeraTK-MotorDriverCard-ApplicationModule 00.02 REQUIRED)
# NOTE The server still depends on the library in order to
# use the dummy motor
add_dependency(mtca4u-MotorDriverCard 01.09 REQUIRED)


IF(ADAPTER STREQUAL "OPCUA")
  message("Building against the OPC UA ControlSystemAdater")
  add_dependency(ChimeraTK-ControlSystemAdapter-OPCUAAdapter REQUIRED)
  set(Adapter_LINK_FLAGS ${ChimeraTK-ControlSystemAdapter-OPCUAAdapter_LINK_FLAGS})
  set(Adapter_LIBRARIES ${ChimeraTK-ControlSystemAdapter-OPCUAAdapter_LIBRARIES})
ELSEIF(ADAPTER STREQUAL "DOOCS")
  message("Building against the DOOCS ControlSystemAdater")
  add_dependency(ChimeraTK-ControlSystemAdapter-DoocsAdapter REQUIRED)
  set(Adapter_LINK_FLAGS ${ChimeraTK-ControlSystemAdapter-DoocsAdapter_LINK_FLAGS})
  set(Adapter_LIBRARIES ${ChimeraTK-ControlSystemAdapter-DoocsAdapter_LIBRARIES})
ELSEIF(ADAPTER STREQUAL "EPICSIOC")
  message("Building against the EPICS IOC ControlSystemAdater")
  add_dependency(ChimeraTK-ControlSystemAdapter-EPICS-IOC-Adapter REQUIRED)
  set(Adapter_LINK_FLAGS ${ChimeraTK-ControlSystemAdapter-EPICS-IOC-Adapter_LINK_FLAGS})
  set(Adapter_LIBRARIES ${ChimeraTK-ControlSystemAdapter-EPICS-IOC-Adapter_LIBRARIES})
ELSE()
  message(FATAL_ERROR "Please select your ControlSystemAdapter to use by passing to the cmake command line:\n"
                      "   -DADAPTER=DOOCS to build a DOOCS server\n"
                      "   -DADAPTER=OPCUA to build an OPC UA server\n"
                      "   -DADAPTER=EPICSIOC to build an EPICS IOC")
ENDIF()

FIND_PACKAGE(Boost COMPONENTS thread system filesystem unit_test_framework REQUIRED)

# extend compiler flags with flags from dependencies
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ChimeraTK-MotorDriverCard-ApplicationModule_CXX_FLAGS}")

set(CMAKE_LINK_FLAGS "-Wl,--no-as-needed ${Adapter_LINK_FLAGS} ${ChimeraTK-ApplicationCore_LINK_FLAGS}" )

# set include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(SYSTEM ${ChimeraTK-MotorDriverCard-ApplicationModule_INCLUDE_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
set(${PROJECT_NAME}_INCLUDE_DIRS ${${PROJECT_NAME}_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include/)


# configure version.h
configure_file(include/version.h.in ${PROJECT_BINARY_DIR}/version/version.h)
include_directories(${PROJECT_BINARY_DIR}/version)

# keep the rpath in the installed executable and library to correctly find the dependencies
if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/export/doocs/server")
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${BINARY_OUTPUT_NAME}/lib")
else()
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

## Library providing the steppermotor ModuleGroup
#file(COPY ${CMAKE_SOURCE_DIR}/cmake/ DESTINATION ${CMAKE_SOURCE_DIR}/steppermotor_module/cmake)
#add_subdirectory(steppermotor_module)
#message("   ** Library headers in ${LIBRARY_HEADER_DIR}")
##set_target_properties(${LIBRARY_OUTPUT_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_FULL_LIBRARY_VERSION}
##                                                    SOVERSION ${${PROJECT_NAME}_SOVERSION})

# main server executable
add_executable(${PROJECT_NAME} src/StepperMotorServer.cc)
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${CMAKE_LINK_FLAGS}")
target_link_libraries(${PROJECT_NAME} ${ChimeraTK-MotorDriverCard-ApplicationModule_LIBRARIES} ${Adapter_LIBRARIES} ${mtca4u-MotorDriverCard_LIBRARIES} ${ChimeraTK-ApplicationCore_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC include ${LIBRARY_HEADER_DIR})

#XML file generation
add_executable(${PROJECT_NAME}-xmlGenerator src/StepperMotorServer.cc)
set_target_properties(${PROJECT_NAME}-xmlGenerator PROPERTIES COMPILE_FLAGS "-DGENERATE_XML")
set_target_properties(${PROJECT_NAME}-xmlGenerator PROPERTIES LINK_FLAGS "${CMAKE_LINK_FLAGS}")
target_link_libraries(${PROJECT_NAME}-xmlGenerator ${ChimeraTK-MotorDriverCard-ApplicationModule_LIBRARIES} ${mtca4u-MotorDriverCard_LIBRARIES} ${ChimeraTK-ApplicationCore_LIBRARIES})
target_include_directories(${PROJECT_NAME}-xmlGenerator PUBLIC include ${LIBRARY_HEADER_DIR})


#add_custom_target(${PROJECT_NAME}-xml ALL DEPENDS ${PROJECT_NAME}-xmlGenerator COMMAND ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-xmlGenerator WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

# Create the executable for automated unit testing.
# add the test executable with all test source files
# TODO Add test
#ENABLE_TESTING()

#aux_source_directory(${CMAKE_SOURCE_DIR}/test test_sources)
#include_directories(${CMAKE_SOURCE_DIR}/test)
#add_executable(test_${PROJECT_NAME} ${test_sources})
#target_link_libraries(test_${PROJECT_NAME} ${ChimeraTK-MotorDriverCard-ApplicationModule_LIBRARIES} ${Adapter_LIBRARIES} ${mtca4u-MotorDriverCard_LIBRARIES} ${Boost_LIBRARIES} ${ChimeraTK-ApplicationCore_LIBRARIES})
#set_target_properties(test_${PROJECT_NAME} PROPERTIES LINK_FLAGS "${CMAKE_LINK_FLAGS}")
#target_include_directories(test_${PROJECT_NAME} PUBLIC include ${LIBRARY_HEADER_DIR})

#add_test(test_${PROJECT_NAME} test_${PROJECT_NAME})


# The make coverage command is only available in debug mode
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
  configure_file(cmake/Makefile.coverage.in ${PROJECT_BINARY_DIR}/Makefile.coverage @ONLY)
  add_custom_target(coverage
    make -f Makefile.coverage
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating test coverage documentation" VERBATIM
  )
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")


FILE( COPY ${CMAKE_SOURCE_DIR}/config/devMapFile.dmap DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/DFMC_MD22_test.map DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/DFMC_MD22_test.mapp DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/newer_firmware_mapfile.map DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/ServerConfiguration.xml DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/motorConfig.xml DESTINATION ${PROJECT_BINARY_DIR})


IF(ADAPTER STREQUAL "DOOCS")
  FILE( COPY ${CMAKE_SOURCE_DIR}/config/${PROJECT_NAME}.conf DESTINATION ${PROJECT_BINARY_DIR})
  #FILE( COPY ${CMAKE_SOURCE_DIR}/config/CTK_StepperMotorServer-DoocsVariableConfig.xml DESTINATION ${PROJECT_BINARY_DIR}) # Do not export until adapted to internal structure
ENDIF()
FILE( COPY ${CMAKE_SOURCE_DIR}/config/initMotorDriverHW.py DESTINATION ${PROJECT_BINARY_DIR})

# Install the executable and the RPC_LIBNO file
if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/export/doocs/server")
  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${PROJECT_NAME}/bin)
  install(TARGETS ${PROJECT_NAME}-xmlGenerator RUNTIME DESTINATION ${PROJECT_NAME}/bin)

  # if no configuration exists, prepare configuration for dummy system
  if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/${PROJECT_NAME}.conf")
    install(FILES config/devMapFile.dmap DESTINATION ${PROJECT_NAME})
    install(FILES config/DFMC_MD22_test.mapp DESTINATION ${PROJECT_NAME})
    install(FILES config/DFMC_MD22_test.map DESTINATION ${PROJECT_NAME})
    
    install(FILES config/ServerConfiguration.xml DESTINATION ${PROJECT_NAME})
    install(PROGRAMS config/initMotorDriverHW.py DESTINATION ${PROJECT_NAME} )
    install(FILES config/${PROJECT_NAME}.conf DESTINATION ${PROJECT_NAME})
    #install(FILES config/${PROJECT_NAME}-DoocsVariableConfig.xml DESTINATION ${PROJECT_NAME}) # Do not export until adapted to internal structure
    install(FILES config/RPC_LIBNO DESTINATION ${PROJECT_NAME})
    install(CODE "
         EXECUTE_PROCESS(COMMAND ln -sf bin/${PROJECT_NAME} ${PROJECT_NAME}
                         WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME})
    ")
  endif()
else()
  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
  install(TARGETS ${PROJECT_NAME}-xmlGenerator RUNTIME DESTINATION bin)
endif()


find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

  add_custom_target(doc ${DOC_DEPENDENCY}
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

  #FIXME At least for Doocs this does not fit
  install(DIRECTORY ${CMAKE_BINARY_DIR}/doc/ DESTINATION share/doc/${PROJECT_NAME}-${${PROJECT_NAME}_SOVERSION} )
else(DOXYGEN_FOUND)
  message("Doxygen not found, documentation cannot be build.")
endif(DOXYGEN_FOUND)

