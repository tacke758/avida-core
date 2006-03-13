SET(ConfigDir "${CMAKE_CURRENT_SOURCE_DIR}/testconfig")
SET(GenesisFile "${ConfigDir}/genesis")
SET(InstSetFile "${ConfigDir}/inst_set.default")
SET(EventFile "${ConfigDir}/events.cfg")
SET(EnvironmentFile "${ConfigDir}/environment.cfg")
SET(StartCreatureFile "${ConfigDir}/kazi.org")
SET(AvidaArgs "-seed %d -set INST_SET ${InstSetFile} -set EVENT_FILE ${EventFile} -set ENVIRONMENT_FILE ${EnvironmentFile} -set START_CREATURE ${StartCreatureFile}")

SET(SeedSet "range(100,110)")
SET(RunSubdirPathBase "${CMAKE_CURRENT_BINARY_DIR}/Kazi_")
SET(OutputFileName "detail_pop.100")
SET(ExpectationFilePathBase "${CMAKE_CURRENT_SOURCE_DIR}/Kazi_")
SET(ExpectationFileName "detail_pop.100.expected_last_line")

SET(CheckTailsScriptTemplate "${PROJECT_SOURCE_DIR}/CMakeModules/Templates/check_tails.py.in") 
SET(CheckTailsScript "${CMAKE_CURRENT_BINARY_DIR}/kazi_check_tails.py") 
CONFIGURE_FILE(${CheckTailsScriptTemplate} ${CheckTailsScript} IMMEDIATE)

IF(AVD_CONSISTENCY_TEST_PRIMITIVE)
  ADD_TEST("GlobalAsex" "${_PythonExePath}" "${CheckTailsScript}")
ENDIF(AVD_CONSISTENCY_TEST_PRIMITIVE)

SET(GenerateTailsScriptTemplate "${PROJECT_SOURCE_DIR}/CMakeModules/Templates/generate_tails.py.in") 
SET(GenerateTailsScript "${CMAKE_CURRENT_BINARY_DIR}/kazi_generate_tails.py") 
CONFIGURE_FILE(${GenerateTailsScriptTemplate} ${GenerateTailsScript} IMMEDIATE)



SET(SeedSet "range(100,110)")
SET(RunSubdirPathBase "${CMAKE_CURRENT_BINARY_DIR}/GlobalAsex_")
SET(OutputFileName "detail_pop.100")
SET(ExpectationFilePathBase "${CMAKE_CURRENT_SOURCE_DIR}/GlobalAsex_")
SET(ExpectationFileName "detail_pop.100.expected_last_line")

SET(CheckTailsScriptTemplate "${PROJECT_SOURCE_DIR}/CMakeModules/Templates/check_tails.py.in") 
SET(CheckTailsScript "${CMAKE_CURRENT_BINARY_DIR}/asex_check_tails.py") 
CONFIGURE_FILE(${CheckTailsScriptTemplate} ${CheckTailsScript} IMMEDIATE)

IF(AVD_CONSISTENCY_TEST_PRIMITIVE)
  ADD_TEST("GlobalAsex" "${_PythonExePath}" "${CheckTailsScript}")
ENDIF(AVD_CONSISTENCY_TEST_PRIMITIVE)

SET(GenerateTailsScriptTemplate "${PROJECT_SOURCE_DIR}/CMakeModules/Templates/generate_tails.py.in") 
SET(GenerateTailsScript "${CMAKE_CURRENT_BINARY_DIR}/asex_generate_tails.py") 
CONFIGURE_FILE(${GenerateTailsScriptTemplate} ${GenerateTailsScript} IMMEDIATE)


