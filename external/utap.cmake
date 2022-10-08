include(FetchContent)


FetchContent_Declare(
  utap
  GIT_REPOSITORY git@github.com:dannybpoulsen/utap.git
  GIT_TAG d07d7688db53cb7e5cdb774a31d55295602507a6
  PATCH_COMMAND sed -i "s/2.9.14 REQUIRED//" ${CMAKE_CURRENT_BINARY_DIR}/_deps/utap-src/CMakeLists.txt
)


FetchContent_MakeAvailable(utap )