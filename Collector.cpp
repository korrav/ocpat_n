/*
 * Collector.cpp
 *
 *  Created on: 07 мая 2015 г.
 *      Author: andrej
 */

#include "Collector.h"
#include <iostream>
#include <algorithm>
#include "Detector.h"

namespace collector_n {
//#define PRINT_INFO_PAKAGE_COLLECTOR
Collector::Collector(): pBuf_(new PtrData::element_type()) {
	pCurMark_ = new mad_n::MarkPack;
	init();
}

void Collector::init(void) {
	pBuf_->clear();
	pCurMark_->id = -1;
	pCurMark_->part = -1;
	pCurMark_->totalPart = -1;
}

std::pair<PtrData, int> Collector::receivBlock(PtrData pDataBuf) {
	if(pDataBuf->size() < sizeof(mad_n::HeadPack)) {
#ifdef PRINT_INFO_PAKAGE_COLLECTOR
		std::cout << "Принят пакет с непозволительно малым объёмом данных" << std::endl;
#endif //PRINT_INFO_PAKAGE_COLLECTOR
		return {nullptr, -1};
	}
	int idBlock = reinterpret_cast<mad_n::HeadPack*>(pDataBuf->data())->endpoints.idBlock;
	mad_n::MarkPack* pMark = reinterpret_cast<mad_n::MarkPack*>(pDataBuf->data() + sizeof(mad_n::SrcPack));
#ifdef PRINT_INFO_PAKAGE_COLLECTOR
	std::cout << "Принят пакет с иден. " << pMark->id << " ,частью " << pMark->part <<" кол. частей " << pMark->totalPart << std::endl;
#endif //PRINT_INFO_PAKAGE_COLLECTOR
	if((pCurMark_->id != pMark->id) || (pMark->part == 0)) {	//пришёл пакет с новым идентификатором блока
		if(pCurMark_->part != -1) {
#ifdef PRINT_INFO_PAKAGE_COLLECTOR
			std::cout << "Потерян незаконченный пакет данных с идент. " << pCurMark_->id << " и частью " << pCurMark_->part << std::endl;
#endif //PRINT_INFO_PAKAGE_COLLECTOR
			init();
		}
		if(pMark->part == 0) {
#ifdef PRINT_INFO_PAKAGE_COLLECTOR
			std::cout << "Принят первый пакет данных с идент. " << pMark->id << std::endl;
#endif //PRINT_INFO_PAKAGE_COLLECTOR
			pCurMark_->id = pMark->id;
			pCurMark_->totalPart = pMark->totalPart;
		} else {
#ifdef PRINT_INFO_PAKAGE_COLLECTOR
			std::cout << "Потерян пакет данных с идент. " << pMark->id << " и частью " << pMark->part << std::endl;
#endif //PRINT_INFO_PAKAGE_COLLECTOR
			init();
			return {nullptr, -1};
		}
	}
	if(++pCurMark_->part != pMark->part) {
//#ifdef PRINT_INFO_PAKAGE_COLLECTOR
		std::cout << "Потерян пакет данных с идент. " << pMark->id << std::endl;
//#endif //PRINT_INFO_PAKAGE_COLLECTOR
		init();
		return {nullptr, -1};
	}
	std::copy(pDataBuf->begin() + sizeof(mad_n::HeadPack),pDataBuf->end(),std::back_inserter(*pBuf_));
	if(pCurMark_->part == pCurMark_->totalPart - 1) {
		PtrData pNewBuf(new PtrData::element_type());
		pBuf_.swap(pNewBuf);
#ifdef PRINT_INFO_PAKAGE_COLLECTOR
		std::cerr << "Пришла последняя часть пакета с идентификатором " << pCurMark_->id << std::endl;
#endif //PRINT_INFO_PAKAGE_COLLECTOR
		init();
		return {pNewBuf, idBlock};
	} else
		return {nullptr, -1};
}

Collector::~Collector() {
	delete pCurMark_;
}

} /* namespace collector_n */
