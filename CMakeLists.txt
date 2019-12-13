# Copyright (c) 2019 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# PDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.0)
project (timedautomata DESCRIPTION "Timed Automata library")

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)

add_library(timedautomata INTERFACE)

install(FILES
    include/discriminator.h
    include/state.h
    include/timed_block_list.h
    include/output_generator.h
    src/discriminator.hpp
    src/output_generator.hpp
  DESTINATION include/timedautomata)
