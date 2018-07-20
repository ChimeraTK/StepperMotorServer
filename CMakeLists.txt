PROJECT(CTK_StepperMotorServer)
cmake_minimum_required(VERSION 2.8)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(${PROJECT_NAME}_MAJOR_VERSION 00)
set(${PROJECT_NAME}_MINOR_VERSION 00)
set(${PROJECT_NAME}_PATCH_VERSION 00)
include(cmake/set_version_numbers.cmake)

# Set names for output binary and library
set(BINARY_OUTPUT_NAME "${PROJECT_NAME}")
set(LIBRARY_OUTPUT_NAME "${PROJECT_NAME}lib")

# Moderate version of the compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -Wextra -ansi -Wuninitialized -O3 -fomit-frame-pointer")

# add constants for HOST and USER
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSER=\\\"`whoami`\\\" -DHOST=\\\"`hostname`\\\"")

#use -DCMAKE_BUILD_TYPE=Debug in your cmake command to turn on the coverage option
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3 --coverage")

# linker flags
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-dynamic -Wl,--no-as-needed")

include(cmake/add_dependency.cmake)
add_dependency(ChimeraTK-ApplicationCore 00.13 REQUIRED)
# TODO which version?
add_dependency(mtca4u-MotorDriverCard 01.04 REQUIRED)

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

## TODO Any use of these?
#add_dependency(mtca4u-VirtualLab 00.06 REQUIRED)
#add_dependency(ChimeraTK-DeviceAccess-DoocsBackend REQUIRED)

FIND_PACKAGE(Boost COMPONENTS thread system filesystem unit_test_framework REQUIRED)
ENABLE_TESTING()

# extend compiler flags with flags from dependencies
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLRFLibrary_CXX_FLAGS} ${ChimeraTK-ApplicationCore_CXX_FLAGS} ${mtca4u-MotorDriverCard_CXX_FLAGS}" )

set(CMAKE_LINK_FLAGS "-Wl,--no-as-needed ${Adapter_LINK_FLAGS} ${ChimeraTK-ApplicationCore_LINK_FLAGS}" )

# set include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(SYSTEM ${mtca4u-MotorDriverCard_INCLUDE_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
set(${PROJECT_NAME}_INCLUDE_DIRS ${${PROJECT_NAME}_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include/)

# create lists with source files
aux_source_directory(${CMAKE_SOURCE_DIR}/src library_sources)

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

# server library (to avoid compiling everyting multiple times)
add_library(${LIBRARY_OUTPUT_NAME} SHARED ${library_sources} )
set_target_properties(${LIBRARY_OUTPUT_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_FULL_LIBRARY_VERSION}
                                                    SOVERSION ${${PROJECT_NAME}_SOVERSION})
target_link_libraries(${LIBRARY_OUTPUT_NAME} ${Boost_LIBRARIES} ${ChimeraTK-ApplicationCore_LIBRARIES} )
#    ${LLRFLibrary_LIBRARIES}  ${mtca4u-VirtualLab_LIBRARIES} )

# main server executable
add_executable(${PROJECT_NAME} src/dummy.cc)
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${CMAKE_LINK_FLAGS}")
target_link_libraries(${PROJECT_NAME} ${LIBRARY_OUTPUT_NAME} ${Adapter_LIBRARIES} ${mtca4u-MotorDriverCard_LIBRARIES})

# TODO: can we use this here? XML file generation
#add_executable(${PROJECT_NAME}-xmlGenerator src/dummy.cc)
#set_target_properties(${PROJECT_NAME}-xmlGenerator PROPERTIES COMPILE_FLAGS "-DGENERATE_XML")
#set_target_properties(${PROJECT_NAME}-xmlGenerator PROPERTIES LINK_FLAGS "${CMAKE_LINK_FLAGS}")
#target_link_libraries(${PROJECT_NAME}-xmlGenerator ${PROJECT_NAME}lib)

#add_custom_target(${PROJECT_NAME}-xml ALL DEPENDS ${PROJECT_NAME}-xmlGenerator COMMAND ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-xmlGenerator WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

# Create the executable for automated unit testing.
# add the test executable with all test source files
# TODO Add test
aux_source_directory(${CMAKE_SOURCE_DIR}/test test_sources)
include_directories(${CMAKE_SOURCE_DIR}/test)
add_executable(test_${PROJECT_NAME} ${test_sources})
target_link_libraries(test_${PROJECT_NAME} ${LIBRARY_OUTPUT_NAME} ${Adapter_LIBRARIES} ${mtca4u-MotorDriverCard_LIBRARIES})
set_target_properties(test_${PROJECT_NAME} PROPERTIES LINK_FLAGS "${CMAKE_LINK_FLAGS}")
add_test(test_${PROJECT_NAME} test_${PROJECT_NAME})

# The make coverage command is only available in debug mode
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
  configure_file(cmake/Makefile.coverage.in ${PROJECT_BINARY_DIR}/Makefile.coverage @ONLY)
  add_custom_target(coverage
    make -f Makefile.coverage
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating test coverage documentation" VERBATIM
  )
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

# TODO Use the DOOCS backend?
#  Copy DOOCSBackend library
#list(GET ChimeraTK-DeviceAccess-DoocsBackend_LIBRARIES 0 DoocsBackendLibSymlink)
#get_filename_component(DoocsBackendLib ${DoocsBackendLibSymlink} REALPATH)
# EXECUTE_PROCESS(COMMAND ln -sf ${DoocsBackendLib} libChimeraTK-DeviceAccess-DoocsBackend.so
#                WORKING_DIRECTORY ${PROJECT_BINARY_DIR})




# TODO: Sort this out later (also: use mapp or map)
FILE( COPY ${CMAKE_SOURCE_DIR}/config/dummies.dmap DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/DFMC_MD22_test.mapp DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/DFMC_MD22_test.map DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/newer_firmware_mapfile.mapp DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/newer_firmware_mapfile.map DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/CTK_StepperMotorConfig.xml DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/motorConfig.xml DESTINATION ${PROJECT_BINARY_DIR})

IF(ADAPTER STREQUAL "OPCUA")
  # TODO Implement [OPCUA]
  FILE( COPY ${CMAKE_SOURCE_DIR}/config/CTK_StepperMotorMapping.xml DESTINATION ${PROJECT_BINARY_DIR})
ELSEIF(ADAPTER STREQUAL "DOOCS")
  FILE( COPY ${CMAKE_SOURCE_DIR}/config/${PROJECT_NAME}.conf DESTINATION ${PROJECT_BINARY_DIR})
  FILE( COPY ${CMAKE_SOURCE_DIR}/config/CTK_StepperMotorServer-DoocsVariableConfig.xml DESTINATION ${PROJECT_BINARY_DIR})
ENDIF()
FILE( COPY ${CMAKE_SOURCE_DIR}/config/initMotorDriverHW.py DESTINATION ${PROJECT_BINARY_DIR})

# Install the executable and the RPC_LIBNO file
if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/export/doocs/server")
  install(TARGETS ${LIBRARY_OUTPUT_NAME} LIBRARY DESTINATION ${PROJECT_NAME}/lib)
  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${PROJECT_NAME}/bin)
  # TODO Remove or adapt
  #install(TARGETS ${PROJECT_NAME}-xmlGenerator RUNTIME DESTINATION llrfCtrl_server/bin)
  #install(FILES ${PROJECT_BINARY_DIR}/libChimeraTK-DeviceAccess-DoocsBackend.so DESTINATION ${PROJECT_NAME})

  # if no configuration exists, prepare configuration for dummy system
  if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/CTK_StepperMotorServer/${PROJECT_NAME}.conf")
    install(FILES config/dummies.dmap DESTINATION ${PROJECT_NAME})
    install(FILES config/DFMC_MD22_test.mapp DESTINATION ${PROJECT_NAME})
    install(FILES config/DFMC_MD22_test.map DESTINATION ${PROJECT_NAME})
    
    # TODO Remove or adapt
    #install(FILES config/${PROJECT_NAME}_controller.xlmap DESTINATION ${PROJECT_NAME})
    #install(FILES config/${PROJECT_NAME}_adc0.xlmap DESTINATION ${PROJECT_NAME})
    install(FILES config/CTK_StepperMotorConfig.xml DESTINATION ${PROJECT_NAME})
    install(PROGRAMS config/initMotorDriverHW.py DESTINATION ${PROJECT_NAME} )
    install(FILES config/${PROJECT_NAME}.conf DESTINATION ${PROJECT_NAME})
    install(FILES config/${PROJECT_NAME}-DoocsVariableConfig.xml DESTINATION ${PROJECT_NAME})
    install(FILES config/RPC_LIBNO DESTINATION ${PROJECT_NAME})
    install(CODE "
         EXECUTE_PROCESS(COMMAND ln -sf bin/${PROJECT_NAME} ${PROJECT_NAME}
                         WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME})
         EXECUTE_PROCESS(COMMAND ln -sf lib/libCTK_StepperMotorServerlib.so .
                         WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME})
     ")
  endif()
else()
  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
  # TODO Remove or adapt
  #install(TARGETS ${PROJECT_NAME}-xmlGenerator RUNTIME DESTINATION bin)
  install(TARGETS ${LIBRARY_OUTPUT_NAME} LIBRARY DESTINATION lib)
endif()

