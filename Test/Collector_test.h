/*
 * Collector_test.h
 *
 *  Created on: 08 мая 2015 г.
 *      Author: andrej
 */

#ifndef COLLECTOR_TEST_H_
#define COLLECTOR_TEST_H_
#include "Detector.h"
#include "Collector.h"
#include <list>
#include <string>
#include <sstream>
#include <memory>
#include <array>
#include <boost/optional.hpp>

//#define PRINT_DEBUG_INFO_COLLECTOR_TEST	//выводить отладочную информацию в процессе тестов

namespace collector_n {

class InfoDistortion {
public:
	virtual const char* what(void) const{
		return "";
	}
	//	virtual std::shared_ptr<InfoDistortion> clone() const {
	//		return std::shared_ptr<InfoDistortion>(new  InfoDistortion());
	//	}
	virtual std::shared_ptr<InfoDistortion> clone() const {
		return std::make_shared<InfoDistortion>(*this);
	}
	virtual ~InfoDistortion(){}
};

class Collector_test {
	typedef void(Collector_test::*funcDistor)(std::list<PtrData>&);
	const int MAX_PAYLOAD_BUFF = 5000;	//максимальный размер полезной нагрузки буфера
	const int NUM_INTERATION = 100000;	//количество тестовых циклов
	const mad_n::SrcPack  ENDPOINTS = { 1, 0};
	PtrData::element_type curBuf_;	//буфер, используемый для анализа коллектора
	std::list<PtrData::element_type> correctListParts_;	//корректный список частей, на которые разбит рассматриваемый буфер
	size_t sizeCorrectListParts_ = 0;
	std::list<PtrData::element_type> incorrectListParts_;	//некорректный список частей, на которые разбит рассматриваемый буфер
	size_t sizeIncorrectListParts_ = 0;
	std::shared_ptr<InfoDistortion> ptrInfoDist_{new InfoDistortion()};	//информация о текущем искажении анализируемого буфера

	PtrData generateRandomBuffer(void);	//генерирует произвольный буфер
	std::pair<std::list<PtrData>, bool> split( PtrData pData);	/*разбивает буфер на фрагменты, иногда примешивая
		к ним некорректный. На выходе вместе с фрагментами сообщается пройдёт ли буфер проверку, либо нет*/
	void distortedWayResort(std::list<PtrData>& slices);	/*искажение коплекта, составляющего буфер, путём
		перетасовки его частей*/
	void distortedWayCutoff(std::list<PtrData>& slices);	/*искажение коплекта, составляющего буфер, путём
		обрезания его конца*/
	void distortedWayChangeuniqNumPart(std::list<PtrData>& slices);	/*искажение коплекта, составляющего буфер, путём
		изменения уникального идентификатора одной из его частей*/
	void distortedWayDubicationPart(std::list<PtrData>& slices);	/*искажение коплекта, составляющего буфер, путём
		дублирования одной из его частей*/
	bool distortArbitrarilyBuffer(std::list<PtrData>& slices);	/*случайным образом искажение частей буфера, true -
		если икажение не произошло*/
	boost::optional<PtrData> merge(std::list<PtrData>& slices, Collector& col);	//возвращает либо объединённый буфер, либо ошибку

	std::array<funcDistor, 4> arrFuncDistor_ = {{&Collector_test::distortedWayResort, &Collector_test::distortedWayCutoff, &Collector_test::distortedWayChangeuniqNumPart,&Collector_test::distortedWayDubicationPart}};
public:
	Collector_test();
	virtual ~Collector_test();

	void setInfoDist(const InfoDistortion& info = InfoDistortion())
	{
		ptrInfoDist_ = info.clone();
	}
	void test(void);
};

class InfoDistWayResort: public InfoDistortion {
	unsigned idxFirstPart_;
	unsigned idxSecondPart_;
public:
	InfoDistWayResort(unsigned idxFirstPart, unsigned idxSecondPart): idxFirstPart_(idxFirstPart), idxSecondPart_(idxSecondPart){}
	virtual const char* what(void) const{
		std::ostringstream mes;
		mes << "Искажение выражено обменом местами частей " << idxFirstPart_ << " и" << idxSecondPart_ << std::endl;
		return mes.str().c_str();
	}
	virtual std::shared_ptr<InfoDistortion> clone() const {
		return std::make_shared<InfoDistWayResort>(*this);
	}
};

class InfoDistWayCutoff: public InfoDistortion {
	unsigned idxPartBorder_;
public:
	InfoDistWayCutoff(unsigned idxPartBorder): idxPartBorder_(idxPartBorder){}
	virtual const char* what(void) const{
		std::ostringstream mes;
		mes << "Искажение выражено обрезанием буфера по границе части " << idxPartBorder_ << std::endl;
		return mes.str().c_str();
	}
	virtual std::shared_ptr<InfoDistortion> clone() const {
		return std::make_shared<InfoDistWayCutoff>(*this);
	}
};

class InfoDistWayChangeuniqNumPart: public InfoDistortion {
	unsigned idxPart_;
public:
	InfoDistWayChangeuniqNumPart(unsigned idxPart): idxPart_(idxPart){}
	virtual const char* what(void) const{
		std::ostringstream mes;
		mes << "Искажение выражено изменением уникального идентификатора части " << idxPart_ << std::endl;
		return mes.str().c_str();
	}
	virtual std::shared_ptr<InfoDistortion> clone() const {
		return std::make_shared<InfoDistWayChangeuniqNumPart>(*this);
	}
};

class InfoDistWayDubicationPart: public InfoDistortion {
	unsigned idxPart_;
public:
	InfoDistWayDubicationPart(unsigned idxPart): idxPart_(idxPart){}
	virtual const char* what(void) const{
		std::ostringstream mes;
		mes << "Искажение выражено дублированием части " << idxPart_ << std::endl;
		return mes.str().c_str();
	}
	virtual std::shared_ptr<InfoDistortion> clone() const {
		return std::make_shared<InfoDistWayDubicationPart>(*this);
	}
};
} /* namespace collector_n */
#endif /* COLLECTOR_TEST_H_ */

