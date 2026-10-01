# Require an explicit failure code, not any crash or runtime-loader failure.
execute_process(COMMAND "${APP}" --project "${PROJECT}" --smoke-test -platform offscreen
  RESULT_VARIABLE code OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
if(NOT "${code}" STREQUAL "6" OR NOT error MATCHES "Open failed:")
  message(FATAL_ERROR "Missing project must report Open failed and exit 6; got ${code}: ${output}${error}")
endif()
execute_process(COMMAND "${APP}" -platform offscreen --project
  RESULT_VARIABLE code OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
if(NOT "${code}" STREQUAL "6" OR NOT error MATCHES "requires a file path")
  message(FATAL_ERROR "Missing path must report usage and exit 6; got ${code}: ${output}${error}")
endif()
