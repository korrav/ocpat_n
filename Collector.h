/*
 * Collector.h
 *
 *  Created on: 07 мая 2015 г.
 *      Author: andrej
 */

#ifndef COLLECTOR_H_
#define COLLECTOR_H_
#include <vector>
#include <memory>
#include <utility>
#include "GeneratorFiles.h"


//#define PRINT_INFO_PAKAGE_COLLECTOR	//печатать информацию о принятых пакетах
namespace mad_n {
struct MarkPack;
}

namespace collector_n {

class Collector {
	PtrData pBuf_;
	mad_n::MarkPack* pCurMark_;
	void init(void);
public:
	Collector();
	std::pair<PtrData, int> receivBlock(PtrData pDataBuf); /*получение блока данных. Если блок данных недоступен,
	то первый параметр nullptr*/
	virtual ~Collector();
};

} /* namespace collector_n */

#endif /* COLLECTOR_H_ */
