add_executable (tt tt.cpp)
target_link_libraries (tt PUBLIC uppaalad)


add_executable (exprwrapper ExprWrapper.cpp)
target_link_libraries (exprwrapper PUBLIC uppaalad doctest)

add_test(exprwrapper exprwrapper)
