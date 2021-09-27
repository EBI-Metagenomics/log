macro(inject_dll_to_please_windows test_name)
  file(TO_CMAKE_PATH "$ENV{PATH}" PATH)
  list(APPEND PATH $<TARGET_FILE_DIR:LOG::log>)
  string(REPLACE ";" "\\;" PATH "${PATH}")
  set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "PATH=${PATH}")
endmacro()

find_package(hope REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

function(log_add_test name srcs)
  add_executable(${name} ${srcs})
  target_link_libraries(${name} PRIVATE LOG::log)
  target_link_libraries(${name} PRIVATE HOPE::hope)
  target_link_libraries(${name} PRIVATE Threads::Threads)
  target_compile_options(${name} PRIVATE ${WARNING_FLAGS})
  target_compile_features(${name} PRIVATE c_std_11)
  add_test(NAME ${name} COMMAND ${name})
  inject_dll_to_please_windows(${name})
  set(TMPDIR "${CMAKE_CURRENT_BINARY_DIR}/${name}.tmp")
  add_custom_command(
    TARGET ${name}
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TMPDIR})
  target_compile_definitions(${name} PUBLIC "TMPDIR=\"${TMPDIR}\"")
endfunction()

log_add_test(threads threads.c)
log_add_test(levels levels.c)
log_add_test(macro macro.c)
