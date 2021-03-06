######################################
# CMakeLists.txt for modules/tvgutil #
######################################

###########################
# Specify the target name #
###########################

SET(targetname tvgutil)

################################
# Specify the libraries to use #
################################

INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseBoost.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseCUDA.cmake)

#############################
# Specify the project files #
#############################

##
SET(boost_headers
include/tvgutil/boost/WrappedAsio.h
)

##
SET(commands_sources
src/commands/Command.cpp
src/commands/CommandManager.cpp
src/commands/NoOpCommand.cpp
src/commands/SeqCommand.cpp
)

SET(commands_headers
include/tvgutil/commands/Command.h
include/tvgutil/commands/CommandManager.h
include/tvgutil/commands/NoOpCommand.h
include/tvgutil/commands/SeqCommand.h
)

##
SET(containers_headers
include/tvgutil/containers/LimitedContainer.h
include/tvgutil/containers/MapUtil.h
include/tvgutil/containers/PooledQueue.h
include/tvgutil/containers/PriorityQueue.h
)

##
SET(filesystem_sources
src/filesystem/FilesystemUtil.cpp
src/filesystem/PathFinder.cpp
src/filesystem/SequentialPathGenerator.cpp
)

SET(filesystem_headers
include/tvgutil/filesystem/FilesystemUtil.h
include/tvgutil/filesystem/PathFinder.h
include/tvgutil/filesystem/SequentialPathGenerator.h
)

##
SET(misc_sources
src/misc/IDAllocator.cpp
src/misc/SettingsContainer.cpp
src/misc/ThreadPool.cpp
)

SET(misc_headers
include/tvgutil/misc/ArgUtil.h
include/tvgutil/misc/AttitudeUtil.h
include/tvgutil/misc/ConversionUtil.h
include/tvgutil/misc/ExclusiveHandle.h
include/tvgutil/misc/IDAllocator.h
include/tvgutil/misc/SettingsContainer.h
include/tvgutil/misc/ThreadPool.h
)

##
SET(net_sources
src/net/AckMessage.cpp
src/net/ClientHandler.cpp
src/net/Message.cpp
)

SET(net_headers
include/tvgutil/net/AckMessage.h
include/tvgutil/net/ClientHandler.h
include/tvgutil/net/Message.h
include/tvgutil/net/Server.h
include/tvgutil/net/SimpleMessage.h
)

##
SET(numbers_sources
src/numbers/RandomNumberGenerator.cpp
)

SET(numbers_headers
include/tvgutil/numbers/NumberSequenceGenerator.h
include/tvgutil/numbers/RandomNumberGenerator.h
)

##
SET(persistence_sources
src/persistence/LineUtil.cpp
src/persistence/PropertyUtil.cpp
)

SET(persistence_headers
include/tvgutil/persistence/LineUtil.h
include/tvgutil/persistence/PropertyUtil.h
include/tvgutil/persistence/SerializationUtil.h
)

##
SET(statistics_headers
include/tvgutil/statistics/Histogram.h
include/tvgutil/statistics/ProbabilityMassFunction.h
)

##
SET(timing_headers
include/tvgutil/timing/AverageTimer.h
include/tvgutil/timing/Timer.h
include/tvgutil/timing/TimeUtil.h
)

#################################################################
# Collect the project files into sources, headers and templates #
#################################################################

SET(sources
${commands_sources}
${filesystem_sources}
${misc_sources}
${net_sources}
${numbers_sources}
${persistence_sources}
)

SET(headers
${boost_headers}
${commands_headers}
${containers_headers}
${filesystem_headers}
${misc_headers}
${net_headers}
${numbers_headers}
${persistence_headers}
${statistics_headers}
${timing_headers}
)

#############################
# Specify the source groups #
#############################

SOURCE_GROUP(boost FILES ${boost_headers})
SOURCE_GROUP(commands FILES ${commands_sources} ${commands_headers})
SOURCE_GROUP(containers FILES ${containers_headers})
SOURCE_GROUP(filesystem FILES ${filesystem_sources} ${filesystem_headers})
SOURCE_GROUP(misc FILES ${misc_sources} ${misc_headers})
SOURCE_GROUP(net FILES ${net_sources} ${net_headers})
SOURCE_GROUP(numbers FILES ${numbers_sources} ${numbers_headers})
SOURCE_GROUP(persistence FILES ${persistence_sources} ${persistence_headers})
SOURCE_GROUP(statistics FILES ${statistics_headers})
SOURCE_GROUP(timing FILES ${timing_headers})

##########################################
# Specify additional include directories #
##########################################

INCLUDE_DIRECTORIES(include/tvgutil)

##########################################
# Specify the target and where to put it #
##########################################

INCLUDE(${PROJECT_SOURCE_DIR}/cmake/SetCUDALibTarget.cmake)

#############################
# Specify things to install #
#############################

INSTALL(TARGETS ${targetname} DESTINATION lib)
