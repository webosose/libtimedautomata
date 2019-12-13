// Copyright (c) 2019 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OUTPUT_GENERATOR_H
#define OUTPUT_GENERATOR_H

#include <chrono>
#include <functional>
#include <thread>

#include <timedautomata/timed_block_list.h>

template<typename Input>
class OutputGenerator
{
public:
    OutputGenerator(TimedBlockList<std::pair<Input, std::chrono::milliseconds>>& output);
    void setNotifyFunction(std::function<void(Input)> f);
    void exit();
    void run();

private:
    TimedBlockList<std::pair<Input, std::chrono::milliseconds>>&  m_output;
    std::chrono::time_point<std::chrono::steady_clock> m_lastSentTime;
    std::function<void(Input)> m_notiFunction;

    bool __running = true;
};

#include "output_generator.hpp"
#endif //OUTPUTGENERATOR_H
