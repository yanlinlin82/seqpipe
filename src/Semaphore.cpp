#include <stdexcept>
#include <sys/stat.h>
#include <fcntl.h>
#include "Semaphore.h"

Semaphore::Semaphore(const std::string& name)
{
	sem_ = sem_open(name.c_str(), O_CREAT | O_CLOEXEC, S_IRUSR | S_IWUSR, 1);
	if (sem_ == SEM_FAILED) {
		perror("sem_open");
		throw std::runtime_error("sem_open");
	}
	name_ = name;
}

Semaphore::~Semaphore()
{
	if (sem_ != SEM_FAILED) {
		sem_close(sem_);
		sem_ = SEM_FAILED;
		sem_unlink(name_.c_str());
		name_.clear();
	}
}

void Semaphore::lock()
{
	sem_wait(sem_);
}

void Semaphore::unlock()
{
	sem_post(sem_);
}
