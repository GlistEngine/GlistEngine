/*
 * Copyright (C) 2016 Nitra Games Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UTILS_GTHREAD_H_
#define UTILS_GTHREAD_H_

#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <chrono>
#include "gObject.h"


class gThread: public gObject {
public:
	gThread();
	virtual ~gThread();

	std::thread::id getId() const;
	bool isRunning();
	bool isCurrent();
	bool isDone();
	std::thread& getThread();

	void start();
	void stop();

	bool lock();
	bool tryLock();
	void unlock();

	void wait();
	void sleep(std::chrono::duration<double, std::milli> milliseconds);
	void sleep(double milliseconds);
    void yield();
    void detach();

    virtual void threadFunction();

protected:
    std::condition_variable conditionvariable;
    mutable std::mutex mutex;
    bool isdone;
    bool isrunning;

private:
    std::thread thread;
    std::chrono::high_resolution_clock::time_point starttime;
    std::chrono::duration<double, std::milli> timediff;

    void run();
};

#endif /* UTILS_GTHREAD_H_ */
