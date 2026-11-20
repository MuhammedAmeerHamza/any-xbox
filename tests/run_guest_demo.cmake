execute_process(
    COMMAND "${PROGRAM}" --play-demo
    INPUT_FILE "${INPUT_FILE}"
    OUTPUT_VARIABLE demo_output
    ERROR_VARIABLE demo_error
    RESULT_VARIABLE demo_result
)

if(NOT demo_result EQUAL 0)
    message(FATAL_ERROR "Guest demo failed with exit code ${demo_result}: ${demo_error}")
endif()

string(FIND "${demo_output}" "Correct! The guest program accepted your answer." success_position)
if(success_position EQUAL -1)
    message(FATAL_ERROR "Guest demo did not report a win. Output: ${demo_output}")
endif()
