add_library (uppaalad INTERFACE)
target_link_libraries (uppaalad INTERFACE UTAP)
target_include_directories (uppaalad INTERFACE ${PROJECT_SOURCE_DIR}/include) 
