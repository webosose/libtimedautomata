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

#include <iostream>

template<typename Input>
OutputGenerator<Input>::OutputGenerator(
    TimedBlockList<std::pair<Input, std::chrono::milliseconds>>& output)
    : m_output(output)
{
}

template<typename Input>
void OutputGenerator<Input>::setNotifyFunction(std::function<void(Input)> f)
{
    m_notiFunction = f;
}

template<typename Input>
void OutputGenerator<Input>::exit()
{
    __running = false;
    m_output.exit();
}

template<typename Input>
void OutputGenerator<Input>::run()
{
    auto& output = m_output;
    while (__running) {
        const auto& [retVal, inputWord] = output.next_element_until();
        if (retVal) {
            const auto& [inputVal, time] = inputWord;
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::steady_clock::now() - m_lastSentTime);

            if (time > diff) {
                std::this_thread::sleep_for(time-diff);
            }

            m_notiFunction(inputVal);
            m_lastSentTime = std::chrono::steady_clock::now();
            output.eraseConsumedPart(); // TODO: Can be pop_front;
                                        // But it needs something like a front_until...
        } else {
            // Critical error...
            std::cerr << "Error: Output buffer returned without value. maybe exit()"
                      << std::endl;
        }
    }
}
