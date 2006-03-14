
# Note :
#
# ${CMAKE_CURRENT_SOURCE_DIR} points to the current source directory,
# i.e., .../Tests/Default/.
#
# ${CMAKE_CURRENT_BINARY_DIR} points to the equivalent binary directory,
# i.e., .../build/Tests/Default/.
#

#
# Avida configuration
#
SET(ConfigDir "${AVIDA_SOURCE_DIR}/source/support")
SET(EventFile "${CMAKE_CURRENT_SOURCE_DIR}/config/events.cfg")
SET(GenesisFile "${ConfigDir}/genesis")
SET(InstSetFile "${ConfigDir}/inst_set.default")
SET(EnvironmentFile "${ConfigDir}/environment.cfg")
SET(StartCreatureFile "${ConfigDir}/organism.default")
SET(AvidaArgs "-seed %d -set WORLD-X 40 -set WORLD-Y 40 -set INST_SET ${InstSetFile} -set EVENT_FILE ${EventFile} -set ENVIRONMENT_FILE ${EnvironmentFile} -set START_CREATURE ${StartCreatureFile}")

#
# Consistency checks configuration
#
SET(TestDirBaseName "Seed")

# This is Python list of Avida random seeds.
# E.g., "range(100,105)" is equivalent to the Python list "[100,101,102,103,104]".
# I.e., SET(SeedSet "range(100,105)") is equivalent to SET(SeedSet "[100,101,102,103,104]").
SET(SeedSet "range(100,105)")
# I've written ${ConfigDir}/events.cfg to stop Avida after 500 updates,
# and to output a single detail-population file "detail_pop.500" per run,
# i.e., ${CMAKE_CURRENT_SOURCE_DIR}/Test_100/detail_pop.500, and so on.
SET(RunLength "500")


SET(OutputFileName "detail_pop.${RunLength}")
# Similar to above, except that this tells the testing scripts where to find
# expected last lines of detail_pop.${RunLength} files,
# i.e., ${CMAKE_CURRENT_SOURCE_DIR}/Test_100/detail_pop.500.expected_last_line, and so on.
SET(ExpectationFileName "detail_pop.${RunLength}.expected_last_line")
# Avida will be run in, and store output files in, ${CMAKE_CURRENT_BINARY_DIR}/Test_100, and so on.
SET(ExpectationFilePathBase "${CMAKE_CURRENT_SOURCE_DIR}/${TestDirBaseName}_")
SET(RunSubdirPathBase "${CMAKE_CURRENT_BINARY_DIR}/${TestDirBaseName}_")

#
# This generates the Python script that performs consistency checks.
# Several of the variables defined above make their ways into the script.
# See CMake's documentaton of the CONFIGURE_FILE command for details.
SET(CheckTailsScriptTemplate "${PROJECT_SOURCE_DIR}/CMakeModules/Templates/check_tails.py.in") 
SET(CheckTailsScript "${CMAKE_CURRENT_BINARY_DIR}/check_tails.py") 
CONFIGURE_FILE(${CheckTailsScriptTemplate} ${CheckTailsScript} IMMEDIATE)

IF(AVD_CONSISTENCY_TEST_PRIMITIVE)
  ADD_TEST("Consistency-Check--Default" "${_PythonExePath}" "${CheckTailsScript}")
ENDIF(AVD_CONSISTENCY_TEST_PRIMITIVE)

#
# Generation of expected last lines of detail_pop.500 files.
#
# To trigger this script, use commands like the following :
#   $ pushd ${CMAKE_CURRENT_BINARY_DIR} # i.e., cd .../build/Tests/Default/
#   $ tar cvfz Tests.tgz `find . -name detail_pop.500.expected_last_line`
#   $ cd ${CMAKE_CURRENT_SOURCE_DIR}
#   $ tar xvfz ${CMAKE_CURRENT_BINARY_DIR}/Tests.tgz
#   $ popd
#
# I think it's safer to require that these steps be performed by hand in order
# to reduce the chance of inadvertently clobbering existing consistency-check suites.
#
SET(GenerateTailsScriptTemplate "${PROJECT_SOURCE_DIR}/CMakeModules/Templates/generate_tails.py.in") 
SET(GenerateTailsScript "${CMAKE_CURRENT_BINARY_DIR}/generate_tails.py") 
CONFIGURE_FILE(${GenerateTailsScriptTemplate} ${GenerateTailsScript} IMMEDIATE)

# Vim modeline to tell Vim that this is a configuration script.
# vim: set ft=config:
