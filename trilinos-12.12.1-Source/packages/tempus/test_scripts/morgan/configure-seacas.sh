
cmake \
-D Trilinos_ENABLE_TESTS:BOOL=OFF \
-D Trilinos_ENABLE_EXAMPLES:BOOL=OFF \
-D Trilinos_ENABLE_DEBUG=OFF \
-D CMAKE_BUILD_TYPE:STRING=RELEASE \
-D Trilinos_ENABLE_SEACAS:BOOL=ON \
-D Trilinos_ENABLE_SEACASIoss:BOOL=ON \
-D TPL_ENABLE_Matio=OFF  \
-D TPL_ENABLE_X11:BOOL=OFF \
-D CMAKE_CXX_COMPILER:FILEPATH="mpicxx" \
-D CMAKE_C_COMPILER:FILEPATH="mpicc" \
-D CMAKE_Fortran_COMPILER:FILEPATH="mpif77" \
-D CMAKE_INSTALL_PREFIX:PATH=${SEACAS_INSTALL_PATH} \
-D Netcdf_INCLUDE_DIRS:FILEPATH="${NETCDF_BASE_DIR}/include" \
-D Netcdf_LIBRARY_DIRS:FILEPATH="${NETCDF_BASE_DIR}/lib" \
-D TPL_Netcdf_LIBRARIES="-L${BOOST_ROOT}/lib;-L${NETCDF_ROOT}/lib;-L${NETCDF_ROOT}/lib;${PNETCDF_ROOT}/lib;-L${HDF5_ROOT}/lib;${BOOST_ROOT}/lib/libboost_program_options.a;${BOOST_ROOT}/lib/libboost_system.a;${NETCDF_ROOT}/lib/libnetcdf.a;${PNETCDF_ROOT}/lib/libpnetcdf.a;${HDF5_ROOT}/lib/libhdf5_hl.a;${HDF5_ROOT}/lib/libhdf5.a;-lz;-ldl" \
-D HDF5_INCLUDE_DIRS:FILEPATH="${HDF_BASE_DIR}/include" \
-D HDF5_LIBRARY_DIRS:FILEPATH="${HDF_BASE_DIR}/lib" \
${WORKSPACE}/Trilinos