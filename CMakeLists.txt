PROJECT(steppermotorserver)
cmake_minimum_required(VERSION 3.16)

set(${PROJECT_NAME}_MAJOR_VERSION 01)
set(${PROJECT_NAME}_MINOR_VERSION 00)
set(${PROJECT_NAME}_PATCH_VERSION 03)
include(cmake/set_version_numbers.cmake)
include(cmake/set_default_build_to_release.cmake)
include(cmake/set_default_flags.cmake)
include(cmake/set_control_system_adapter.cmake)
include(cmake/enable_code_style_check.cmake)

find_package(ChimeraTK-ApplicationCore 03.00 REQUIRED)
find_package(ChimeraTK-MotorDriverCard-ApplicationModule 00.07 REQUIRED)
# NOTE: The server still depends on the library in order to
# use the dummy motor
find_package(mtca4u-MotorDriverCard 01.13 REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread system filesystem unit_test_framework)

# configure version.h
configure_file(include/version.h.in ${PROJECT_BINARY_DIR}/version/version.h)
include_directories(${PROJECT_BINARY_DIR}/version)

# Make object file of the Application's "main" module available
add_library(objStepperMotorServer OBJECT src/StepperMotorServer.cc src/MotorDummy.cc include/MotorDummy.h
                                         include/StepperMotorServer.h)
target_include_directories(objStepperMotorServer PRIVATE include)
target_link_libraries(objStepperMotorServer ChimeraTK::ChimeraTK-MotorDriverCard-ApplicationModule
                                            ChimeraTK::SelectedAdapter
                                            ChimeraTK::mtca4u-MotorDriverCard
                                            ChimeraTK::ChimeraTK-ApplicationCore)

# main server executable
add_executable(${PROJECT_NAME} src/ApplicationInstance.cc $<TARGET_OBJECTS:objStepperMotorServer>)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} ChimeraTK::ChimeraTK-MotorDriverCard-ApplicationModule
                                      ChimeraTK::SelectedAdapter
                                      ChimeraTK::mtca4u-MotorDriverCard
                                      ChimeraTK::ChimeraTK-ApplicationCore)

#XML file generation
add_executable(${PROJECT_NAME}-xmlGenerator src/ApplicationInstance.cc $<TARGET_OBJECTS:objStepperMotorServer>)
target_compile_definitions(${PROJECT_NAME}-xmlGenerator PRIVATE GENERATE_XML=1)
target_include_directories(${PROJECT_NAME}-xmlGenerator PRIVATE include)
target_link_libraries(${PROJECT_NAME}-xmlGenerator ChimeraTK::ChimeraTK-MotorDriverCard-ApplicationModule
                                                   ChimeraTK::SelectedAdapter
                                                   ChimeraTK::mtca4u-MotorDriverCard
                                                   ChimeraTK::ChimeraTK-ApplicationCore)

# Create the executable for automated unit testing.
# add the test executable with all test source files
ENABLE_TESTING()

aux_source_directory(${CMAKE_SOURCE_DIR}/tests/executables_src testExecutables)
include_directories(${CMAKE_SOURCE_DIR}/test)

foreach( testExecutableSrcFile ${testExecutables})
  #NAME_WE means the base name without path and (longest) extension
  get_filename_component(executableName ${testExecutableSrcFile} NAME_WE)
  add_executable(${executableName} ${testExecutableSrcFile})
  target_sources(${executableName} PRIVATE $<TARGET_OBJECTS:objStepperMotorServer>)
  target_include_directories(${executableName} PRIVATE include)
  target_link_libraries(${executableName} ChimeraTK::ChimeraTK-MotorDriverCard-ApplicationModule ChimeraTK::SelectedAdapter
      ChimeraTK::mtca4u-MotorDriverCard ChimeraTK::ChimeraTK-ApplicationCore Boost::unit_test_framework)
  add_test(${executableName} ${executableName})
endforeach( testExecutableSrcFile )


# enable code coverate report
include(cmake/enable_code_coverage_report.cmake)


FILE( COPY ${CMAKE_SOURCE_DIR}/config/devMapFile.dmap DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/DFMC_MD22_test.map DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/DFMC_MD22_test.mapp DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/newer_firmware_mapfile.map DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/ServerConfiguration.xml DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/motorConfig.xml DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/config/motorProxy.xlmap DESTINATION ${PROJECT_BINARY_DIR})


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

include(cmake/enable_doxygen_documentation.cmake)
