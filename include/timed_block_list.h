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

#ifndef TIMED_BLOCK_LIST_H
#define TIMED_BLOCK_LIST_H

#include <list>
#include <mutex>
#include <condition_variable>

using namespace std::chrono_literals;

template <typename T> class TimedBlockList
{
    std::list<T> list_;
    int32_t currentLoc_ = 0;
    int32_t maxLoc = 0;

    std::mutex mutex_;
    std::condition_variable condVar_;

    bool updated = false;
    bool shouldExit_ = false;

    typedef std::lock_guard<std::mutex> lock;


    using RetVal = std::pair<bool, T>;

public:
    bool isUpdated() { return updated; }
    // TODO: need to reset iter.begin() position.
    // If we can redefine begin() position, we can remove currentLoc.
    void reset() { updated = false; }
    void exit() { shouldExit_= true; condVar_.notify_one(); }
    void push_back(const T& val)
    {
        lock l(mutex_);
        bool wake = list_.size() < currentLoc_ + 1;
        list_.push_back(val);
        if (wake) condVar_.notify_one();
        updated = true;
    }

    void push_back(T&& val) // universal reference
    {
        lock l(mutex_); // prevents multiple pushes corrupting queue_
        bool wake = list_.size() < currentLoc_ + 1;
        list_.push_back(std::forward<T>(val));
        if (wake) condVar_.notify_one();
        updated = true;
    }

    void insert_back(typename std::list<T>::iterator begin, typename std::list<T>::iterator end)
    {
        lock l(mutex_);
        bool wake = list_.size() < currentLoc_ + 1;
        list_.insert(list_.end(), begin, end);
        if (wake) condVar_.notify_one();
    }

    std::pair<typename std::list<T>::iterator, typename std::list<T>::iterator>
            getConsumedPart()
    {
        lock l(mutex_);
        auto iTer = list_.begin(); // TODO: make iTer a member variable
        advance(iTer, maxLoc);

        return {list_.begin(), iTer};
    }

    RetVal next_element_until(std::chrono::milliseconds ms = 0ms)
    {
        std::unique_lock<std::mutex> u(mutex_);

        if (list_.size() < currentLoc_ + 1) { // there's no data yet.
            if (ms == 0ms) { // no time-out case
                condVar_.wait(u, [&list = list_, &currentLoc = currentLoc_, &shouldExit = shouldExit_]() { return (list.size() >= currentLoc + 1 || shouldExit); });
            } else if (!condVar_.wait_for(u, ms, [&list = list_, &currentLoc = currentLoc_, &shouldExit = shouldExit_]() { return (list.size() >= currentLoc + 1 || shouldExit); })) {
                return {false, T()};  // time-out!, early return...
            }
        }

        if (shouldExit_)
            return {false, T()};

        // Now list_ is non-empty and we still have the LOCK!!!
        auto iTer = list_.begin();  // TODO: make iTer member variable
        advance(iTer, currentLoc_++);
        if (maxLoc < currentLoc_)
            maxLoc = currentLoc_;

        return {true, *iTer};
    }

    void rewindCurrentLoc() {
        lock l(mutex_);
        currentLoc_ = 0;
    }

    void eraseConsumedPart() {
        lock l(mutex_);
        auto iTer = list_.begin();
        advance(iTer, maxLoc);
        currentLoc_ = 0;
        maxLoc = 0;
        list_.erase(list_.begin(), iTer);
    }
};

#endif //TIMED_BLOCK_LIST_H
