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

#include <type_traits>
#include <timedautomata/output_generator.h>

template <typename T,
          typename std::enable_if_t<!std::is_pointer<T>::value>* = nullptr,
          typename IterStart, typename IterEnd>
inline void deleter(T val, IterStart start, IterEnd end) {  }

template <typename T,
          typename std::enable_if_t<std::is_pointer<T>::value>* = nullptr,
          typename IterStart, typename IterEnd>
inline void deleter(T val, IterStart start, IterEnd end) {
    for (auto begin = start; begin!=end; begin++)
        delete (*begin).first;
}

template<typename Input>
Discriminator<Input>::~Discriminator()
{
    delete m_processThread;
    delete m_outputGeneratorThread;

    // Now we can delete init state and output
    for (auto& [key, value] : env) {
        delete value; //delete environment variable.
        delete key;   //delete init state.
    }
    delete m_outputGenerator;
}

template<typename Input>
void Discriminator<Input>::init()
{
    __running = true;
    m_outputGenerator = new OutputGenerator<Input>(outputList);
    m_processThread = new std::thread(&Discriminator<Input>::process, this);
    m_outputGeneratorThread = new std::thread(&OutputGenerator<Input>::run, m_outputGenerator);
}

template<typename Input>
void Discriminator<Input>::finalize()
{
    __running = false;
    inputList.exit();
    m_outputGenerator->exit();

    m_processThread->join();
    m_outputGeneratorThread->join();
}

template<typename Input>
void Discriminator<Input>::setNotifyFunction(std::function<void(Input)> f)
{
    if (m_outputGenerator) {
        m_outputGenerator->setNotifyFunction(f);
    }
}

template<typename Input>
void Discriminator<Input>::addInitState(State<Input>* state) {
    // Environment variables are reused on the whole life
    // cycle of the state chains.
    auto envVar = state->getEnvVar();
    if (!envVar) {
        state->setEnvVar((env[state] = new DFAEnv()));
    } else { // User can set their Environemnt variable
        env[state] = envVar;
    }
    state->setOutput(&outputList);
    m_initialStates.push_back(state);
}

template<typename Input>
void Discriminator<Input>::process()
{
    bool recognized = false;
    while (__running) {
        for (auto& initState : m_initialStates) {
            m_currentState = initState;
            if ((recognized = processState())) {
                outputList.reset();
                break;
            }
            inputList.rewindCurrentLoc();
        }

        auto [begin, end] = inputList.getConsumedPart();
        if (!recognized) { // no machine accepts strings so far...
            outputList.insert_back(begin, end);
        } else { // if the input is pushed to output, we don't need to delete it.
            // only if Type is a pointer, delete it.
            deleter((*begin).first, begin, end);
        }
        inputList.eraseConsumedPart();
        m_lastInputTime.reset();
    }
}

template<typename Input>
bool Discriminator<Input>::processState()
{
    while (m_currentState) {
        State<Input>* state = m_currentState->handleInput(inputList); // can be blocked.
        if (m_currentState != state) {
            auto it = std::find(m_initialStates.begin(), m_initialStates.end(), m_currentState);
            if (it == m_initialStates.end()) // TODO: Consider shared_ptr?
                delete m_currentState;
            m_currentState = state;
        }
    }
    return outputList.isUpdated();
}

template<typename Input>
void Discriminator<Input>::pushInput(const Input& input)
{
    auto currentInputTime = std::chrono::steady_clock::now();
    std::chrono::milliseconds diff;
    if (m_lastInputTime) {
        diff = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentInputTime - *m_lastInputTime);
    } else {
        diff = std::chrono::milliseconds::zero();
    }

    m_lastInputTime = currentInputTime;
    inputList.push_back(std::make_pair(input, diff));
}
