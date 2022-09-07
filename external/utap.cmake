include(FetchContent)


FetchContent_Declare(
  utap
  GIT_REPOSITORY git@github.com:dannybpoulsen/utap.git
  GIT_TAG 25099f64d7c6371c7b4e0591e430b11191e9728a
  PATCH_COMMAND sed -i "s/2.9.14 REQUIRED//" ${CMAKE_CURRENT_BINARY_DIR}/_deps/utap-src/CMakeLists.txt
)


FetchContent_MakeAvailable(utap )