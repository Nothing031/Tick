#pragma once
#include <chrono>
#include <thread>
#include <time.h>
#include <ctime>
#include <condition_variable>
#include <mutex>

class Tick
{
private:
	enum TickMode {
		T_Null,
		T_Cv,
		T_CvFlag,
		T_CvFlagDouble,
		T_Flag,
		T_FlagDouble
	};
	const int _advancedTimer_microsecond = 100;
	int _interval;
	bool* _pExternalFlag1;
	bool* _pExternalFlag2;
	TickMode _mode;
	std::condition_variable* _pExternalCV;
	std::thread _thread;

	void SetMode() {
		if (this->_pExternalCV != nullptr) {
			if (this->_pExternalFlag1 != nullptr || this->_pExternalFlag2 != nullptr){
				if (this->_pExternalFlag1 == nullptr) {
					this->_pExternalFlag1 = this->_pExternalFlag2;
					this->_mode = TickMode::T_CvFlag;
					return;
				}
				if (this->_pExternalFlag2 == nullptr) {
					this->_mode = TickMode::T_CvFlag;
					return;
				}
				this->_mode = TickMode::T_CvFlagDouble;
				return;
			}
			this->_mode = TickMode::T_Cv;
			return;
		}
		else {
			if (this->_pExternalFlag1 != nullptr || this->_pExternalFlag2 != nullptr) {
				if (this->_pExternalFlag1 == nullptr) {
					this->_pExternalFlag1 = this->_pExternalFlag2;
					this->_mode = TickMode::T_Flag;
					return;
				}
				if (this->_pExternalFlag2 == nullptr) {
					this->_mode = TickMode::T_Flag;
					return;
				}
				this->_mode = TickMode::T_FlagDouble;
				return;
			}
			this->_mode = TickMode::T_Null;
			return;
		}
	}
	void Init(int interval, std::condition_variable* pExternalCv, bool* pExternalFlag1, bool* pExternalFlag2) {
		this->_interval = interval;
		this->tickCount = 0;
		this->running = false;
		this->_pExternalCV = pExternalCv;
		this->_pExternalFlag1 = pExternalFlag1;
		this->_pExternalFlag2 = pExternalFlag2;
		SetMode();
	}

	void _CV() {
		int start = clock();
		int now;
		while (running) {
			now = clock();
			if (now - start >= this->_interval) {
				start = clock();
				this->_pExternalCV->notify_one();
			}
			std::this_thread::sleep_for(std::chrono::microseconds(_advancedTimer_microsecond));
		}
	}
	void _CVFLAG() {
		int start = clock();
		int now;
		while (running) {
			now = clock();
			if (now - start >= this->_interval) {
				start = clock();
				*this->_pExternalFlag1 = true;
				this->_pExternalCV->notify_one();
			}
			std::this_thread::sleep_for(std::chrono::microseconds(_advancedTimer_microsecond));
		}
	}
	void _CVFLAGDOUBLE() {
		int start = clock();
		int now;
		while (running) {
			now = clock();
			if (now - start >= this->_interval) {
				start = clock();
				*this->_pExternalFlag1 = true;
				*this->_pExternalFlag2 = true;
				this->_pExternalCV->notify_one();
			}
			std::this_thread::sleep_for(std::chrono::microseconds(_advancedTimer_microsecond));
		}
	}
	void _FLAG() {
		int start = clock();
		int now;
		while (running) {
			now = clock();
			if (now - start >= this->_interval) {
				start = clock();
				*this->_pExternalFlag1 = true;
			}
			std::this_thread::sleep_for(std::chrono::microseconds(_advancedTimer_microsecond));
		}
	}
	void _FLAGDOUBLE() {
		int start = clock();
		int now;
		while (running) {
			now = clock();
			if (now - start >= this->_interval) {
				start = clock();
				*this->_pExternalFlag1 = true;
				*this->_pExternalFlag2 = true;
			}
			std::this_thread::sleep_for(std::chrono::microseconds(_advancedTimer_microsecond));
		}
	}

public:
	int tickCount;
	bool running;
	Tick(int interval, std::condition_variable* pExternalCv, bool* pExternalFlag1, bool* pExternalFlag2) {
		this->Init(interval, pExternalCv, pExternalFlag1, pExternalFlag2);
	}
	Tick(const Tick& other) {
		this->Init(other._interval, other._pExternalCV, other._pExternalFlag1, other._pExternalFlag2);
	}
	Tick operator=(const Tick& other) {
		this->Init(other._interval, other._pExternalCV, other._pExternalFlag1, other._pExternalFlag2);
	}

	~Tick() {
		this->Stop();
	}
	void Start() {
		this->running = true;
		switch (this->_mode) {
		case TickMode::T_Cv: this->_thread = std::thread(&Tick::_CV, this); return;
		case TickMode::T_CvFlag: this->_thread = std::thread(&Tick::_CVFLAG, this); return;
		case TickMode::T_CvFlagDouble: this->_thread = std::thread(&Tick::_CVFLAGDOUBLE, this); return;
		case TickMode::T_Flag: this->_thread = std::thread(&Tick::_FLAG, this); return;
		case TickMode::T_FlagDouble: this->_thread = std::thread(&Tick::_FLAGDOUBLE, this); return;
		case TickMode::T_Null: this->running = false; return;
		}
	}
	void Stop() {
		this->running = false;
		if (this->_thread.joinable())
			this->_thread.join();
	}
};

