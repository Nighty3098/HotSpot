# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/HotSpot_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/HotSpot_autogen.dir/ParseCache.txt"
  "HotSpot_autogen"
  )
endif()
