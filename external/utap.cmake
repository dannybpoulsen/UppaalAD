include(FetchContent)


FetchContent_Declare(
  utap
  GIT_REPOSITORY https://github.com/UPPAALModelChecker/utap
  GIT_TAG 3103d35680a54c9a6acb9c75fd55488c09da809c
  PATCH_COMMAND sed -i "s/2.9.14 REQUIRED//" ${CMAKE_CURRENT_BINARY_DIR}/_deps/utap-src/CMakeLists.txt
)


FetchContent_MakeAvailable(utap )