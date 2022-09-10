FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)


add_executable (main main.cpp)
target_link_libraries (main PUBLIC uppaalad ${Boost_LIBRARIES})
