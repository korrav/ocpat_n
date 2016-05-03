/*
 * Detector.h
 *
 *  Created on: 17 февр. 2014 г.
 *      Author: andrej
 */
#ifndef DETECTOR_H_
#define DETECTOR_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <map>
#include <mutex>
#include <vector>
#include <memory>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_array.hpp>
#include "Oscillograph.h"
#include "Collector.h"
#include "StorageMad.h"

namespace mad_n {
/*
 *класс обслуживания модуля акустического детектора
 */

struct MarkPack {	//метка пакета
	int id;	//уникальный номер блока данных
	int part;	//порядковый номер куска блока данных
	int totalPart;	//общее количество частей в пакете
};
struct SrcPack {
	int idSrc;	//идентификатор источника
	int idBlock;	//идентификатор блока
};

struct HeadPack {	//заголовок пакета
	SrcPack endpoints;
	MarkPack mark;
};

class Detector {
	enum id_block {	//идентификаторы блоков данных
		DATA,
		COMMAND,
		ANSWER,
		INFO
	};
public:
	enum id_data {
		MONITOR,
		FILTER,
		GASIK,
		CONTINIOUS
	};
private:

	enum id_info {
		OVERLOAD //сообщение о перегрузке каналов МАД
	};

	struct DataAlgorithm { //структура данных, в которую оборачиваются результаты алгоритмов по поиску нейтрино
		unsigned int numFirstCount; //номер первого отсчёта
		int8_t data; //первый байт данных
	};

	struct h_pack_ans { //структура, содержащая ответ на команду
		int id; //идентификатор команды
		int status; //результат выполнения команды (OK или NOT_OK)
	};

	enum status_ans {
		NOT_OK, OK
	};


	/*СТРУКТУРЫ ПАКЕТОВ ДАННЫХ*/

	//DATA

	//мониторограмма
	/*
	 * 1)struct HeadPack
	 * 2)struct GenFiles::Head
	 * 3)struct GenMonitorFile::Monitor
	 */
	//алгоритм фильтрации нейтрино
	/*
	 * 1)struct HeadPack
	 * 2)struct GenFiles::Head
	 * 3)struct GenFilterAlgFile::Params
	 * 4)struct mad_n::DataAlgorithm
	 * 5)char data2
	 * 6)**********
	 * 7)char dataN
	 */
	//алгоритм ГАСИК
	/*
	 * 1)struct HeadPack
	 * 2)struct GenFiles::Head
	 * 3)struct GenGasikFile::Params
	 * 4)struct mad_n::DataAlgorithm
	 * 5)char data2
	 * 6)**********
	 * 7)char dataN
	 */
	//алгоритм непрерывной передачи данных
	/*
	 * 1)struct HeadPack
	 * 2)struct GenFiles::Head
	 * 3)struct mad_n::DataAlgorithm
	 * 4)char data2
	 * 5)**********
	 * 6)char dataN
	 */

	//INFO

	/*
	 * 1)struct HeadPack
	 * 2)payload
	 */

	//ANSWER
	/*
	 * 1)struct HeadPack
	 * 2)struct h_pack_ans
	 * 3)payload
	 */

	//COMMAND
	/*
	 * 1)struct HeadPack
	 * 2)int id_command
	 * 3)arg1
	 * ****************
	 * 4)argN
	 */

	enum id_command {
		ID_START_ADC, //запустить АЦП преобразование
		ID_STOP_ADC, //остановить АЦП преобразование
		ID_SET_GAIN, //установить коэффициенты усиления
		ID_START_TEST, //начать тестирование драйвера АЦП
		ID_STOP_TEST, //завершить тестирование драйвера АЦП
		OPEN_ALG, //запустить алгоритм
		CLOSE_ALG, //остановить алгоритм
		CLOSE_ALL_ALG, //закрыть все действующие алгоритмы
		GET_AC, //получить информацию о действующих в данный момент алгоритмах
		SYNC, //синхронизация АЦП
		SET_PERIOD_MONITOR, //установить период передачи мониторограмм
		GET_PERIOD_MONITOR, //получить период передачи мониторограмм
		/*специализированные команды*/
		//алгоритм gas
		SET_SIGMA, //установить коэффициент превышения шума
		GET_SIGMA, //получить текущее значение коэффициента превышение шума
		SET_PB, //установить параметры блока данных, передаваемого на БЦ
		GET_PB, //получить параметры блока данных, передаваемого на БЦ
		SET_PERIOD_MONITOR_PGA,	//установить новый период мониторинга pga
		START_GASIK, //запуск сеанса измерения ГАСИК
		STOP_GASIK, //остановка сеанса измерения ГАСИК

	};

	std::map<std::string, int> alg_;

	static const int MAX_SIZE_PACK = 1400;	//максимальный размер передаваемого UDP пакета
	const int SAMPLE_PERIOD = 1894;	//период выборки (нс)
	static int sock_;
	static boost::posix_time::ptime syncTime_; //время последней синхронизации
	static std::mutex mutSyncTime_; //мьютекс, защищающий обращение к времени последней синхронизации
	static std::mutex mutTrans_; //мьютекс, защищающий передачу данных
	mutable sockaddr_in addrMad_;
	int idM_; //идентификатор МАД
	static int idB_; //идентификатор БЭГ
	bool isEnableWriteData_ = true; //разрешение производить запись данных
	bool isEnableShowMonit_; //разрешение показывать на экране информацию о принятой мониторограмме
	static sockaddr_in addrBroad_; //широковещательный адрес подсетки МАД
	const static int PeriodOfSync = 300; //период отправки синхросигнала (сек)
	osc_n::Oscillograph osc_;	//осциллограф
	std::shared_ptr<genfiles_n::StorageManager> pStorManager_;	//менеджер хранилищ
	collector_n::Collector collector_;	//сборщик принятых пакетов
	static int idPack_;	//идентификатор передаваемого пакета данных

	static void automatic_generation_of_sync(void); //процедура автоматической генерации синхросигналов через заданные промежутки времени
	void transCom(void *pbuf, size_t size) const; //передача команды в МАД

	void handlingData(PtrData pPack);	//обработка блока, содержащего данные для записи
	void handlingMonitor(PtrData pPack, int idStor);	//обработка принятых мониторограмм
	void monitortoScreen(const genfiles_n::GenFiles::Head& head, const genfiles_n::GenMonitorFile::Monitor& monit) const; /* показать
		мониторограмму на экране*/
	void handlingFilter(PtrData pPack, int idStor);		//обработка данных алгоритма FILTER
	void handlingGasik(PtrData pPack, int idStor);		//обработка данных алгоритма GASIK
	void handlingContinious(PtrData pPack);				//обработка данных алгоритма CONTINIOUS
	void writeDataInOsc(PtrData pData);	//передача данных в программу осциллографа
	void handlingInfo(PtrData pPack); //обработка сообщений МАД
	void handlingCommandsReply(PtrData pPack); //обработка ответов на команды

	static boost::posix_time::ptime get_sync_time(void); //получение времени последней синхронизации
	static std::string get_sync_time_Str(void); //получение времени (строка) последней синхронизации
	static int getIdPack(void);	//возвратить идентификатор пакета данных, предназначенного для передачи
public:
	static void set_sync_time(boost::posix_time::ptime time); //установка времени последней синхронизации
	void receive(PtrData pPack); //приём буфера для обработки
	void get_sigma(void) const; //запросить коэффициент превышения порога шума
	void get_set_algorithm(void) const; //запросить список действующих в данный момент алгоритмов
	void set_period_monitor(const unsigned& s) const; //установить период передачи мониторограмм (секунды)
	void get_period_monitor(void) const; //получить период передачи мониторограмм (секунды)
	void start_gasik(void); //запуск сеанса Гасик
	void stop_gasik(void); //остановка сеанса Гасик
	static void start_automatic_generation_of_sync(void); //запуск процедуры автоматической генерации синхросигналов
	void enable_write_data(void); //разрешить записывать принимаемые данные
	void disable_write_data(void); //запретить записывать принимаемые данные
	void enable_view_monitor(void); //разрешить показывать информацию мониторограмм
	void disable_view_monitor(void); //запретить показывать информацию мониторограмм
	static void set_broadcast_address(const sockaddr_in& addr); //установка широковещательного адреса для подсетки МАД
	static void set_sock(const int& sock); //установка сокета для обмена сообщениями по ethernet
	static void set_src_id(const int& id); //установка идентификатора машины берегового центра
	static void sync(void); //синхронизация всех МАД
	void single_sync(void);	//отправка синхросигнала одному МАД
	void start_adc(void) const; //запуск АЦП преобразования
	void stop_adc(void) const; //остановка АЦП преобразования
	void start_test(void) const; //запуск тестового преобразования
	void stop_test(void) const; //остановка тестового преобразования
	void set_gain(int* gain) const; //установка коэффициентов усиления
	void open_alg(const std::string& alg) const; //включить алгоритм
	void close_alg(const std::string& alg) const; //выключить алгоритм
	void close_all_alg(void) const; //выключить все алгоритмы
	void set_parameter_block(const unsigned int& bef,
			const unsigned int& after) const; //установить параметры блока данных, передаваемого на БЦ
	void get_parameter_block(void) const; //получить параметры блока данных, передаваемого на БЦ
	void set_sigma(const int& s) const; //установить коэффициент превышения шума
	void set_period_monitoring_pga(unsigned char period);	//установка периода мониторинга деятельности pga; 0 - мониторинг отключен
	void view_oscillograph(void);	//показать осциллограф
	void close_oscillograph(void);	//закрыть осциллограф
	Detector(const char* ip, unsigned short port, const int& im, std::shared_ptr<genfiles_n::StorageManager> pStor);
	virtual ~Detector();
};

inline int Detector::getIdPack(void) {
	if(++idPack_ >= std::numeric_limits<int>::max())
		idPack_ = 0;
	return idPack_;
}

} /* namespace mad_n */

#endif /* DETECTOR_H_ */
