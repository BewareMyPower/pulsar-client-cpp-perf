#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#

cmake_minimum_required(VERSION 3.10)
project(protobuf-demo CXX)

if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif ()

find_path(PULSAR_INCLUDE_DIR pulsar/Client.h)
find_library(PULSAR_LIBRARIES pulsar)
message(STATUS "PULSAR_INCLUDE_DIR: ${PULSAR_INCLUDE_DIR}")
message(STATUS "PULSAR_LIBRARIES: ${PULSAR_LIBRARIES}")

if (NOT PULSAR_INCLUDE_DIR OR NOT PULSAR_LIBRARIES)
    message(FATAL_ERROR "Failed to find libpulsar")
endif ()

find_package(Boost REQUIRED COMPONENTS program_options)
message(STATUS "Boost_PROGRAM_OPTIONS_LIBRARY: ${Boost_PROGRAM_OPTIONS_LIBRARY}")

find_package(Threads REQUIRED)
MESSAGE(STATUS "Threads library: " ${CMAKE_THREAD_LIBS_INIT})

include_directories(${PULSAR_INCLUDE_DIR})

add_executable(perfProducer PerfProducer.cc)
target_link_libraries(perfProducer PRIVATE ${PULSAR_LIBRARIES} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
