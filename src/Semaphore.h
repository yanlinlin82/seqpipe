#pragma once
#include <string>
#include <mutex>
#include <semaphore.h>

class Semaphore
{
public:
	explicit Semaphore(const std::string& name);
	~Semaphore();

	Semaphore(const Semaphore&) = delete;
	void operator = (const Semaphore&) = delete;
public:
	void lock();
	void unlock();
private:
	sem_t* sem_;
	std::string name_;
};
