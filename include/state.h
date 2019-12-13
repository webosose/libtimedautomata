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

#ifndef STATE_H
#define STATE_H

#include <any>
#include <map>

#include <timedautomata/timed_block_list.h>

using DFAEnv = std::map<std::string, std::any>;

template<typename T>
class State {
public:
    State() = default;
    State(DFAEnv* env, TimedBlockList<std::pair<T, std::chrono::milliseconds>>* output) {
        this->env = env; this->output = output; }
    virtual ~State() = default;
    virtual State<T>* handleInput(TimedBlockList<std::pair<T, std::chrono::milliseconds>>&) = 0;

    void setEnvVar(DFAEnv* env) { this->env = env; }
    void setOutput(TimedBlockList<std::pair<T, std::chrono::milliseconds>>* output) { this->output = output; }

protected:
    // When you use a pointer as environement variable,
    // the ownership of the allocated object belongs to the user.
    // Make sure that the object to be deleted after use in that case.
    DFAEnv* env = nullptr;
    TimedBlockList<std::pair<T, std::chrono::milliseconds>>* output = nullptr;
};

#endif //STATE_H
