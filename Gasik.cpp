/*
 * Gasik.cpp
 *
 *  Created on: 26 мая 2015 г.
 *      Author: andrej
 */

#include "Gasik.h"
#include <chrono>
#include <thread>

using namespace boost::asio;

namespace gasik_n {

Gasik::Gasik(const char* ip, unsigned short port, int sock): sock_(sock) {
	bzero(&addrPcGasik_, sizeof(addrPcGasik_));
	addrPcGasik_.sin_family = AF_INET;
	addrPcGasik_.sin_port = htons(port);
	inet_pton(AF_INET, ip, &addrPcGasik_.sin_addr);
}

void Gasik::addCallFuncAtStart(std::function<void(void)> callF) {
	funcsAtStart_.push_back(callF);
}

void Gasik::addCallFuncAtStop(std::function<void(void)> callF) {
	funcsAtStop_.push_back(callF);
}

void Gasik::start(void) {	//
	std::lock_guard<std::mutex> lk(mut_);
	if(!is_start_gasik_) {
		for (auto& func : funcsAtStart_)
			func();
		is_start_gasik_ = true;
		ioservice.reset();
		timer_.expires_from_now(std::chrono::seconds{DuratinSeanseGasik});
		timer_.async_wait([this](const boost::system::error_code &ec) { stop(); });
		std::thread t{[this](){ ioservice.run(); }};
		t.detach();
	}
}

void Gasik::stop(void) {
	std::lock_guard<std::mutex> lk(mut_);
	if(is_start_gasik_) {
		for (auto& func : funcsAtStop_)
			func();
		is_start_gasik_ = false;
		timer_.cancel();
	}
}

void Gasik::receivMessage(PtrData pBuf) {
	if(pBuf->size() != sizeof(int))
		return;
	int* pCommand = reinterpret_cast<int*>(pBuf->data());
	switch(*pCommand) {
	case COM_START:
		start();
		break;
	case COM_STOP:
		stop();
	}
}

Gasik::~Gasik() {
}

} /* namespace gasik_n */
