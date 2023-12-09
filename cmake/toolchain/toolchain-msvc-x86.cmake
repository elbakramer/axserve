# set target system processor first
set(CMAKE_SYSTEM_PROCESSOR "x86")

# then include the auto version to handle that
include("${CMAKE_CURRENT_LIST_DIR}/toolchain-msvc-auto.cmake")
