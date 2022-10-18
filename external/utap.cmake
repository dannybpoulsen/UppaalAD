include(FetchContent)


FetchContent_Declare(
  utap
  GIT_REPOSITORY git@github.com:dannybpoulsen/utap.git
  GIT_TAG  5e30a9c288f3345fc2abb4ec55f2e31efdfae697
  PATCH_COMMAND sed -i "s/2.9.14 REQUIRED//" ${CMAKE_CURRENT_BINARY_DIR}/_deps/utap-src/CMakeLists.txt
)


FetchContent_MakeAvailable(utap )