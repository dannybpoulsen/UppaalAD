include(FetchContent)


FetchContent_Declare(
  utap
  GIT_REPOSITORY git@github.com:dannybpoulsen/utap.git
  GIT_TAG 991b80737328660c4cbf7a0d7841f1ec44dcad69
  PATCH_COMMAND sed -i "s/2.9.14 REQUIRED//" ${CMAKE_CURRENT_BINARY_DIR}/_deps/utap-src/CMakeLists.txt
)


FetchContent_MakeAvailable(utap )