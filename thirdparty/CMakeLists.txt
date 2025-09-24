
include("Ccache.cmake")

# Tolk accessibility support - include directly here for now
if(WIN32)
	option(SRB2_CONFIG_HAVE_TOLK "Enable Tolk screen reader support" ON)
	
	if(SRB2_CONFIG_HAVE_TOLK)
		message(STATUS "Tolk accessibility support enabled for Windows")
		add_library(tolk INTERFACE)
		target_include_directories(tolk INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tolk)
		target_compile_definitions(tolk INTERFACE HAVE_TOLK)
	endif()
else()
	message(STATUS "Tolk is only available on Windows, using stub implementation")
	add_library(tolk STATIC ${CMAKE_CURRENT_SOURCE_DIR}/tolk/tolk_stub.c)
	target_include_directories(tolk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tolk)
endif()

# Main CMakeLists.txt should have added our directory to the path
