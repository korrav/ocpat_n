/*
 * Oscillograph.cpp
 *
 *  Created on: 14 марта 2015 г.
 *      Author: andrej
 */

#include "Oscillograph.h"
#include <algorithm>
#include <memory>
#include <iostream>

namespace osc_n {

Oscillograph::Oscillograph(uint64_t samplPeriod, int index) :
		dtime_(samplPeriod) {
	NAME_EXE_FILE += std::to_string(index);
}

bool Oscillograph::start(void) {
	file_ = popen(NAME_EXE_FILE.c_str(), "w");
	if (file_ != nullptr) {
		return true;
	} else
		return false;
}

void Oscillograph::stop(void) {
	if (file_ != nullptr)
		pclose(file_);
}

void Oscillograph::write(std::array<uint16_t, 4> buf) {
	if (!isActive())
		return;
	auto pBuf = generateData(buf);
	if (fwrite(pBuf.get(), sizeof(Data), 1, file_) == EOF)
		stop();
}

bool Oscillograph::isActive(void) {
	return file_ != nullptr;
}

std::shared_ptr<Oscillograph::Data> Oscillograph::generateData(
		std::array<uint16_t, 4> data) {
	Data *pBuf = new Data;
	pBuf->dtime = dtime_;
	std::copy(data.begin(), data.end(), pBuf->data);
	return std::shared_ptr<Data>(pBuf);
}

Oscillograph::~Oscillograph() {
	stop();
}

} /* namespace osc */
