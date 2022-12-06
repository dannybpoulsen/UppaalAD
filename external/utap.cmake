include(FetchContent)


FetchContent_Declare(
  utap
  GIT_REPOSITORY git@github.com:dannybpoulsen/utap.git
  GIT_TAG  cdf5adc7f29ff91e3176a128af6a730cd46183bb
  PATCH_COMMAND sed -i "s/2.9.14 REQUIRED//" ${CMAKE_CURRENT_BINARY_DIR}/_deps/utap-src/CMakeLists.txt
)


FetchContent_MakeAvailable(utap )