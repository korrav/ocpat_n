/*
 * Collector_test.cpp
 *
 *  Created on: 07 мая 2015 г.
 *      Author: andrej
 */

#include "Collector_test.h"
#include <GenFiles_test.h>
#include <Collector_test.h>
#include <random>
#include <chrono>
#include <algorithm>
#include <utility>
#include <iterator>
#include <boost/assert.hpp>
#include <assert.h>

namespace collector_n {

Collector_test::Collector_test() {

}

PtrData Collector_test::generateRandomBuffer(void) {
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine dre(seed);
	std::uniform_int_distribution<unsigned int> dri(sizeof(mad_n::HeadPack),sizeof(mad_n::HeadPack) + MAX_PAYLOAD_BUFF);
	unsigned int size = dri(dre);
	PtrData pBuf(new PtrData::element_type(size));
	mad_n::HeadPack* pHead = reinterpret_cast<mad_n::HeadPack*>(pBuf->data());
	pHead->endpoints = ENDPOINTS;
	pHead->mark.id = dri(dre);
	std::iota(pBuf->begin() + sizeof(mad_n::HeadPack), pBuf->end(), 1);
	curBuf_ = *pBuf;
	return pBuf;
}

std::pair<std::list<PtrData>, bool> Collector_test::split(PtrData pData) {
	unsigned int numAllElem = pData->size() - sizeof(mad_n::HeadPack);
	unsigned int numAllPart;
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine dre(seed);
	//определяем случайным образом количество частей, на которые будет разбит буфер
	if(numAllElem == 0)
		numAllPart = 1;
	else {
		std::uniform_int_distribution<unsigned int> driNumPart(1,pData->size() - sizeof(mad_n::HeadPack));
		numAllPart = driNumPart(dre);
	}
	mad_n::HeadPack* pHeadCurBuf = reinterpret_cast<mad_n::HeadPack*>(curBuf_.data());
	pHeadCurBuf->mark.totalPart = numAllPart;
	mad_n::HeadPack* pHeadBuf = reinterpret_cast<mad_n::HeadPack*>(pData->data());
	pHeadBuf->mark.totalPart = numAllPart;
	//создаём случайным образом вектор, определяющий размеры частей буфера
	std::vector<unsigned int> sizePart;
	if(numAllPart == 1)
		sizePart.push_back(numAllElem);
	else {
		unsigned numRoundsElem = 0;
		for(unsigned part = 0; part < numAllPart - 1; part++) {
			int numElemInPart = (numAllElem - numRoundsElem) / (numAllPart - part);
			std::uniform_int_distribution<unsigned int> driNumElem(0,numElemInPart);
			numElemInPart = driNumElem(dre);
			sizePart.push_back(numElemInPart);
			numRoundsElem  += numElemInPart;
		}
		sizePart.push_back(numAllElem - numRoundsElem);
	}
	//разбиваем буфер на части
	std::list<PtrData>  listPartBuf;
	unsigned numRoundsElem = 0;
	for(unsigned int i = 0; i < sizePart.size(); i++) {
		listPartBuf.emplace_back(new PtrData::element_type(sizeof(mad_n::HeadPack) + sizePart[i]));
		auto posBeginData = std::copy_n(pData->begin(), sizeof(mad_n::HeadPack), listPartBuf.back()->begin());
		std::copy_n(pData->begin() + sizeof(mad_n::HeadPack) + numRoundsElem, sizePart[i], posBeginData);
		numRoundsElem += sizePart[i];
		mad_n::HeadPack* pHead = reinterpret_cast<mad_n::HeadPack*>(listPartBuf.back()->data());
		pHead->mark.part = i;
	}
	correctListParts_.clear();
	std::transform(listPartBuf.begin(), listPartBuf.end(),std::back_inserter(correctListParts_), [](PtrData pD)
			{return *pD;});
	sizeCorrectListParts_ = correctListParts_.size();
	//произвольно искажаем комплект частей, из которых состоит буфер
	bool isCorrect = true;
	if(listPartBuf.size() > 1)
		isCorrect = distortArbitrarilyBuffer(listPartBuf);
	return std::make_pair(std::move(listPartBuf), isCorrect);
}

void Collector_test::distortedWayResort(std::list<PtrData>& slices) {
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine dre(seed);
	std::uniform_int_distribution<unsigned int> driNumElements(0, slices.size() - 1);
	unsigned numFirstElem = driNumElements(dre), numSecondElem;
	do {
		numSecondElem = driNumElements(dre);
	} while (numSecondElem == numFirstElem);
	auto posFirstElem =  slices.begin();
	std::advance(posFirstElem, numFirstElem);
	auto posSecondElem = slices.begin();
	std::advance(posSecondElem, numSecondElem);
	std::iter_swap(posFirstElem, posSecondElem);
	setInfoDist(InfoDistWayResort(numFirstElem, numSecondElem));
}

void Collector_test::distortedWayCutoff(std::list<PtrData>& slices) {
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine dre(seed);
	std::uniform_int_distribution<unsigned int> driBorderCut(1, slices.size() - 1);
	unsigned border = driBorderCut(dre);
	slices.resize(border);
	setInfoDist(InfoDistWayCutoff(border));
}

void Collector_test::distortedWayChangeuniqNumPart(std::list<PtrData>& slices) {
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine dre(seed);
	std::uniform_int_distribution<unsigned int> driNumElements(1, slices.size() - 1);
	unsigned numElem = driNumElements(dre);
	auto posElem =  slices.begin();
	std::advance(posElem, numElem);
	mad_n::HeadPack* pHead = reinterpret_cast<mad_n::HeadPack*>(posElem->get()->data());
	pHead->mark.id++;
	setInfoDist(InfoDistWayChangeuniqNumPart(numElem));
}

void Collector_test::distortedWayDubicationPart(std::list<PtrData>& slices) {
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine dre(seed);
	std::uniform_int_distribution<unsigned int> driNumElements(1, slices.size() - 1);
	unsigned numElem = driNumElements(dre);
	auto posElem =  slices.begin();
	std::advance(posElem, numElem);
	slices.insert(posElem, *posElem);
	setInfoDist(InfoDistWayDubicationPart(numElem));
}

bool Collector_test::distortArbitrarilyBuffer(std::list<PtrData>& slices) {
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine dre(seed);
	std::bernoulli_distribution driIsCorrect(0.75);
	bool is_correct = driIsCorrect(dre);
	if(is_correct) {
		setInfoDist();
	} else {
		std::uniform_int_distribution<unsigned int> driDistor(0, arrFuncDistor_.size() - 1);
		(this->*arrFuncDistor_[driDistor(dre)])(slices);
	}
	incorrectListParts_.clear();
	std::transform(slices.begin(), slices.end(),std::back_inserter(incorrectListParts_), [](PtrData pD)
			{return *pD;});
	sizeIncorrectListParts_ = incorrectListParts_.size();
	return is_correct;
}

boost::optional<PtrData> Collector_test::merge(std::list<PtrData>& slices, Collector& col) {
	std::pair<PtrData, int> buf;
	for(auto pSlic : slices)
		buf = col.receivBlock(pSlic);
	if(buf.first  == nullptr)
		return boost::optional<PtrData>();
	else
		return buf.first;

}
void Collector_test::test(void) {
	PtrData pBuffer;	//испытуемый буфер
	std::list<PtrData> slices;	//содержит фрагменты, на которые разбивается буфер данных
	bool is_valid;		//корректно ли производилась дефрагментация буфера данных
	Collector col;	//тестируемый коллектор
	for(int i = 0; i < NUM_INTERATION; i++) {
		pBuffer = generateRandomBuffer();
		std::tie(slices, is_valid) = split(pBuffer);
#ifdef PRINT_DEBUG_INFO_COLLECTOR_TEST
		mad_n::HeadPack* pHead = reinterpret_cast<mad_n::HeadPack*>(curBuf_.data());
		std::cout << "Пакет с идент. " << pHead->mark.id << " разбит на " << slices.size() << " частей" << std::endl;
		std::cout << "Искажение " << (is_valid? "отсутствует" : std::string("присутствует: ") + ptrInfoDist_->what()) << std::endl;
#endif //PRINT_DEBUG_INFO_COLLECTOR_TEST
		auto returnBuf = merge(slices, col);
		const char* info = ptrInfoDist_->what();
		if(!returnBuf)
			BOOST_ASSERT_MSG(!is_valid, info);
		else {
			BOOST_ASSERT_MSG(is_valid, info);
			if(returnBuf.get()->size()  !=  curBuf_.size() - sizeof(mad_n::HeadPack)) {
#ifdef PRINT_DEBUG_INFO_COLLECTOR_TEST
				std::cout << "Размер полученного буфера " << returnBuf.get()->size() <<
				" не совпадает с исходным размером " << curBuf_.size() - sizeof(mad_n::HeadPack) << std::endl;
#endif //PRINT_DEBUG_INFO_COLLECTOR_TEST
			}
			assert(returnBuf.get()->size()  ==  curBuf_.size() - sizeof(mad_n::HeadPack));
			assert(std::equal(curBuf_.begin() + sizeof(mad_n::HeadPack), curBuf_.end(),returnBuf.get()->begin()));
		}
#ifdef PRINT_DEBUG_INFO_COLLECTOR_TEST
		std::cout << "Тестирование буфера с иден. " << pHead->mark.id <<" прошло успешно\n";
#endif //PRINT_DEBUG_INFO_COLLECTOR_TEST
	}
#ifdef PRINT_DEBUG_INFO_COLLECTOR_TEST
	std::cout << "Тестирование коллектора прошло успешно\n";
#endif //PRINT_DEBUG_INFO_COLLECTOR_TEST
}

Collector_test::~Collector_test() {
}

} /* namespace collector_n */
