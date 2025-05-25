if ("${CMAKE_VERSION}" VERSION_GREATER "3.0.2")
	add_library(header INTERFACE)
	target_include_directories(header INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
endif ()
