#pragma once
#include <thread>
#include <string>
#include <functional>

void setThreadName(std::thread& t, const std::wstring& name);

template<typename... Args>
class ThreadWrapper
{
public:
	ThreadWrapper(const std::string& name, std::function<void(Args...)> func, Args... args)
		: _running(true),
		_thread([func, this, args...](){ while (_running){ func(args...); }})
	{
		setThreadName(_thread, { name.begin(), name.end() });
	}

	void markForShutDown()
	{
		_running = false;
	}

	void join()
	{
		if (_thread.joinable())
		{
			_thread.join();
		}
	}

	~ThreadWrapper()
	{
		markForShutDown();
		join();
	}

private:
	std::thread _thread;
	std::atomic<bool> _running;
};
