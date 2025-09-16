/*
 * gThread.cpp
 *
 *  Created on: May 10, 2021
 *      Author: noyan
 */

#include "gThread.h"


gThread::gThread() {
	isrunning = false;
	isdone = false;
}

gThread::~gThread() {
	// We ask to stop and wait for it to complete.
	stop();
	wait();
}

std::thread::id gThread::getId() const {
	return thread.get_id();
}

bool gThread::isRunning() {
	return isrunning;
}

bool gThread::isCurrent() {
	return getId() == std::this_thread::get_id();
}

bool gThread::isDone() {
	return isdone;
}

std::thread& gThread::getThread() {
	return thread;
}

void gThread::start() {
	if(isrunning){
		gLogw("gThread") << "Thread is already running!";
		return;
	}

	isrunning = true;
	isdone = false;
	thread = std::thread(&gThread::run, this);
}

void gThread::stop() {
	isrunning = false;
}

bool gThread::lock() {
	mutex.lock();
	return true;
}

bool gThread::tryLock() {
	return mutex.try_lock();
}

void gThread::unlock() {
	if(isCurrent()) return;

	mutex.unlock();
	std::unique_lock<std::mutex> lockmutex(mutex);
	conditionvariable.wait(lockmutex);
	isrunning = false;
}

void gThread::wait() {
	if(!thread.joinable()) {
		return;
	}
	thread.join();
}

void gThread::yield() {
	std::this_thread::yield();
}

void gThread::detach() {
	thread.detach();
}

void gThread::sleep(std::chrono::duration<double, std::milli> milliseconds) {
	starttime = std::chrono::high_resolution_clock::now();
	while(true) {
		timediff = std::chrono::high_resolution_clock::now() - starttime;
		if(timediff >= milliseconds) break;
	}
}

void gThread::sleep(double milliseconds) {
	sleep(std::chrono::duration<double, std::milli>{milliseconds});
}

void gThread::run() {
	try {
		threadFunction();
	} catch(const std::exception& exc) {
		gLoge("gThread") << exc.what();
	}

	isrunning = false;
	isdone = true;
}

void gThread::threadFunction() {

}
