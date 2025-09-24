# Tolk library for screen reader accessibility support
# Only enabled on Windows platforms

if(WIN32)
	option(SRB2_CONFIG_HAVE_TOLK "Enable Tolk screen reader support" ON)
	
	if(SRB2_CONFIG_HAVE_TOLK)
		# Look for system-installed Tolk first
		find_path(TOLK_INCLUDE_DIR tolk.h)
		find_library(TOLK_LIBRARY tolk)
		
		if(TOLK_INCLUDE_DIR AND TOLK_LIBRARY)
			message(STATUS "Found system Tolk library")
			add_library(tolk INTERFACE IMPORTED)
			target_include_directories(tolk INTERFACE ${TOLK_INCLUDE_DIR})
			target_link_libraries(tolk INTERFACE ${TOLK_LIBRARY})
		else()
			message(STATUS "System Tolk not found, will use bundled headers")
			# Create interface library for Tolk
			add_library(tolk INTERFACE)
			target_include_directories(tolk INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tolk)
			# Tolk is header-only on Windows, dynamically loads screen readers
		endif()
		
		target_compile_definitions(tolk INTERFACE HAVE_TOLK)
	endif()
else()
	message(STATUS "Tolk is only available on Windows, using stub implementation")
	# Create library with stub implementation for non-Windows platforms
	add_library(tolk STATIC ${CMAKE_CURRENT_SOURCE_DIR}/tolk/tolk_stub.c)
	target_include_directories(tolk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tolk)
endif()