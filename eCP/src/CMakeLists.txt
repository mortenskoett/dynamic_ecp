# Not super good and only for CMake 3.12 or later:
#file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${ECP_SOURCE_DIR}/include/eCP/*.hpp")
#file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${ECP_SOURCE_DIR}/src/*.cpp")
#add_library(eCP_library SHARED ${SOURCE_LIST} ${HEADER_LIST})

# -- Shared Library --
# Not super good and only for CMake 3.12 or later:
file(GLOB_RECURSE SHARED_SOURCE_FILES CONFIGURE_DEPENDS
  ${ECP_SOURCE_DIR}/src/*.cpp
	${ECP_SOURCE_DIR}/include/eCP/*.hpp
)

add_library(eCP_library
  SHARED
  ${SHARED_SOURCE_FILES}
)

target_include_directories(eCP_library
    PUBLIC 
        ./
        ../include)

target_compile_options(eCP_library
    PUBLIC
        ${cflags}
        -O3
        -g
				-fPIC
)

target_link_libraries(eCP_library
  PRIVATE
    HDF5::HDF5
)
