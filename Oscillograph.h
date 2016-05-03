/*
 * Oscillograph.h
 *
 *  Created on: 14 марта 2015 г.
 *      Author: andrej
 */

#ifndef OSCILLOGRAPH_H_
#define OSCILLOGRAPH_H_
#include <string>
#include <stdio.h>
#include <array>
#include <memory>
namespace osc_n {

class Oscillograph {
	std::string NAME_EXE_FILE = "/home/andrej/workspace_qt/osc/source/simpl_oscillograph/build-simpl_oscillograph-Desktop_Qt_5_4_1_GCC_64bit-Debug/osc -t=16 -n 4 -r -b mad";	//имя программы осциллографа
	FILE* file_ = nullptr;	//указатель на исполняемый файл osc
	struct Data {
		uint64_t dtime;
		int16_t data[4];
	};
	uint64_t dtime_;

	std::shared_ptr<Data> pData;
	std::shared_ptr<Oscillograph::Data> generateData(std::array<uint16_t, 4> data);	//генерирование буфера данных для передачи в канал osc
public:
	bool isActive(void);	//запущена ли программа осциллографа
	bool start(void);	//запуск осцилллографа
	void stop(void);	//остановка осциллографа
	void write(std::array<uint16_t, 4> buf);	//передать данные в осциллограф
	Oscillograph(uint64_t samplPeriod, int index);
	virtual ~Oscillograph();
};

} /* namespace osc */

#endif /* OSCILLOGRAPH_H_ */
