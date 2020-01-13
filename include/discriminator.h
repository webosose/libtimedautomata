// Copyright (c) 2019-2020 LG Electronics, Inc.
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

#ifndef DISCRIMINATOR_H
#define DISCRIMINATOR_H

#include <any>
#include <chrono>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <optional>
#include <utility>

#include <timedautomata/state.h>
#include <timedautomata/timed_block_list.h>
#include <timedautomata/output_generator.h>

// Discrimiantor starts with init() and stops with finalize().
// User should delete this only after call finalize().
template<typename Input>
class Discriminator {
public:
    virtual ~Discriminator();

    void init();
    void finalize();
    void addInitState(State<Input>* state);
    void process();

    // Owership of the pushed input belongs to this discriminator.
    // When input stream is destroyed all the objects belong to this stream
    // will be deleted, INCLUDING POINTER TYPE!.
    // If states want to use this input, make sure that YOU HAVE COPIED THAT OBJECT.
    void pushInput(const Input&);
    bool processState();

    void setNotifyFunction(std::function<void(Input)>);

    TimedBlockList<std::pair<Input, std::chrono::milliseconds>>& getOutput() { return outputList; }

protected:
    State<Input>* m_currentState = nullptr;
    std::list<State<Input>*> m_initialStates;

    std::optional<std::chrono::time_point<std::chrono::steady_clock>> m_lastInputTime;

    TimedBlockList<std::pair<Input, std::chrono::milliseconds>> inputList;
    TimedBlockList<std::pair<Input, std::chrono::milliseconds>> outputList;
    OutputGenerator<Input>* m_outputGenerator = nullptr;

    std::map<State<Input>*, DFAEnv*> env;
    bool __running = true;

private:
    std::thread* m_processThread = nullptr;
    std::thread* m_outputGeneratorThread = nullptr;
};

#include "discriminator.hpp"
#endif //DISCRIMINATOR_H
