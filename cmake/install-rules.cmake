install(
    TARGETS vast-checker_exe
    RUNTIME COMPONENT vast-checker_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
