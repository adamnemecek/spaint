##################################
# CMakeLists.txt for modules/orx #
##################################

###########################
# Specify the target name #
###########################

SET(targetname orx)

################################
# Specify the libraries to use #
################################

INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseBoost.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseCUDA.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseEigen.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseInfiniTAM.cmake)

#############################
# Specify the project files #
#############################

##
SET(base_sources
src/base/MemoryBlockFactory.cpp
)

SET(base_headers
include/orx/base/MemoryBlockFactory.h
include/orx/base/ORImagePtrTypes.h
include/orx/base/ORMemoryBlockPtrTypes.h
)

##
SET(geometry_sources
src/geometry/GeometryUtil.cpp
)

SET(geometry_headers
include/orx/geometry/DualNumber.h
include/orx/geometry/DualQuaternion.h
include/orx/geometry/GeometryUtil.h
include/orx/geometry/Screw.h
)

##
SET(relocalisation_sources
src/relocalisation/NullRelocaliser.cpp
src/relocalisation/RefiningRelocaliser.cpp
src/relocalisation/Relocaliser.cpp
)

SET(relocalisation_headers
include/orx/relocalisation/NullRelocaliser.h
include/orx/relocalisation/RefiningRelocaliser.h
include/orx/relocalisation/Relocaliser.h
)

IF(WITH_CUDA)
  SET(relocalisation_sources ${relocalisation_sources} src/relocalisation/BackgroundRelocaliser.cpp)
  SET(relocalisation_headers ${relocalisation_headers} include/orx/relocalisation/BackgroundRelocaliser.h)
ENDIF()

#################################################################
# Collect the project files into sources, headers and templates #
#################################################################

SET(sources
${base_sources}
${geometry_sources}
${relocalisation_sources}
)

SET(headers
${base_headers}
${geometry_headers}
${relocalisation_headers}
)

#############################
# Specify the source groups #
#############################

SOURCE_GROUP(base FILES ${base_sources} ${base_headers})
SOURCE_GROUP(geometry FILES ${geometry_sources} ${geometry_headers})
SOURCE_GROUP(relocalisation FILES ${relocalisation_sources} ${relocalisation_headers})

##########################################
# Specify additional include directories #
##########################################

INCLUDE_DIRECTORIES(include/orx)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/modules/tvgutil/include)

##########################################
# Specify the target and where to put it #
##########################################

INCLUDE(${PROJECT_SOURCE_DIR}/cmake/SetCUDALibTarget.cmake)

#############################
# Specify things to install #
#############################

INSTALL(TARGETS ${targetname} DESTINATION lib)
