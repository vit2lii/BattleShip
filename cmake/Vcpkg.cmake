if(${PROJECT_NAME}_ENABLE_VCPKG)
  #
  # If `vcpkg.cmake`  does not exist, download it.
  #
  if(NOT EXISTS "${CMAKE_BINARY_DIR}/vcpkg.cmake")
    message(
      STATUS
      "Downloading dependency helper..."
    )
    file(
      DOWNLOAD "https://github.com/microsoft/vcpkg/raw/master/scripts/buildsystems/vcpkg.cmake"
      "${CMAKE_BINARY_DIR}/vcpkg.cmake"
    )
    message(STATUS "Vcpkg config downloaded succesfully.")
  endif()

	if(${PROJECT_NAME}_VERBOSE_OUTPUT)
		set(VCPKG_VERBOSE ON)
	endif()
	set(CMAKE_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}" "${CMAKE_BINARY_DIR}/vcpkg.cmake")
endif()
