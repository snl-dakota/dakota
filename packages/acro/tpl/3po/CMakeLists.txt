add_library(3po
  cobyla2c/cobyla.c
  hooke/hooke_tpl.cpp
)
install(TARGETS 3po EXPORT ${ExportTarget} DESTINATION lib)
install(FILES cobyla2c/cobyla.h DESTINATION include/3po)
