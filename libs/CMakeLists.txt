add_library (uppaalad STATIC uppaalad.cpp)
target_link_libraries (uppaalad PUBLIC UTAP)
target_include_directories (uppaalad PUBLIC ${PROJECT_SOURCE_DIR}/include) 
