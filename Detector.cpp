/*
 * Detector.cpp
 *
 *  Created on: 17 февр. 2014 г.
 *      Author: andrej
 */

#include "Detector.h"
#include <string.h>
#include <sys/stat.h>
#include <thread>
#include <iomanip>
#include <array>
#include <algorithm>
#include <functional>
#include "GeneratorFiles.h"
#include <boost/format.hpp>
#include <stddef.h>
#include <limits>
#include <cmath>
#include <iostream>

using std::string;
using std::cout;

namespace mad_n {
sockaddr_in Detector::addrBroad_;
int Detector::sock_;
int Detector::idB_;
boost::posix_time::ptime Detector::syncTime_;
std::mutex Detector::mutSyncTime_;
std::mutex Detector::mutTrans_;
int Detector::idPack_ = 0;

static string find_name_algorithm(const int& id,
		const std::map<string, int>& m); //найти название алгоритма по идентификатору(если нет -возвращает пустую строку)

std::string getTimeStr(const boost::posix_time::ptime& time) {
	std::stringstream ss;
	boost::posix_time::time_facet* pFacet = new boost::posix_time::time_facet("%d%m%y_%H%M%S");
	ss.imbue(std::locale{ss.getloc(), pFacet});
	ss << time;
	return ss.str();
}
Detector::Detector(const char* ip, unsigned short port, const int& im, std::shared_ptr<genfiles_n::StorageManager> pStor) :
				idM_(im), isEnableShowMonit_(false), osc_(SAMPLE_PERIOD, im), pStorManager_(pStor) {
	bzero(&addrMad_, sizeof(addrMad_));
	addrMad_.sin_family = AF_INET;
	addrMad_.sin_port = htons(port);
	inet_pton(AF_INET, ip, &addrMad_.sin_addr);

	alg_.insert(std::make_pair("cont", CONTINIOUS));
	alg_.insert(std::make_pair("gas", GASIK));
	alg_.insert(std::make_pair("filter", FILTER));
}

void Detector::start_adc(void) const {
	int buf = ID_START_ADC;
	transCom(&buf, sizeof(buf));
	return;
}

void Detector::stop_adc(void) const {
	int buf = ID_STOP_ADC;
	transCom(&buf, sizeof(buf));
	return;
}

void Detector::start_test(void) const {
	int buf = ID_START_TEST;
	transCom(&buf, sizeof(buf));
	return;
}

void Detector::stop_test(void) const {
	int buf = ID_STOP_TEST;
	transCom(&buf, sizeof(buf));
	return;
}

void Detector::set_gain(int* gain) const {
	int buf[5] = { ID_SET_GAIN };
	for (int i = 1; i < 5; i++)
		buf[i] = gain[i - 1];
	transCom(&buf, sizeof(buf));
}

void Detector::transCom(void* pbuf, size_t size) const {
	//получение идентификатора передаваемого пакета
	static std::array<int8_t, MAX_SIZE_PACK> buf;
	const int MAX_SIZE_PAYLOAD_PACK = MAX_SIZE_PACK - sizeof(mad_n::HeadPack);
	static mad_n::HeadPack* pHead = reinterpret_cast<mad_n::HeadPack*>(&buf[0]);
	static auto pData = buf.begin() + sizeof(mad_n::HeadPack);

	int8_t* pTransBuf = reinterpret_cast<int8_t*>(pbuf);
	//из скольких частей состоит пакет
	int numPart = ceil(static_cast<double>(size)/ MAX_SIZE_PAYLOAD_PACK);
	//заполнение шапки пакета
	pHead->endpoints.idSrc = idB_;
	pHead->endpoints.idBlock = COMMAND;
	mutTrans_.lock();
	pHead->mark.id = getIdPack();
	mutTrans_.unlock();
	pHead->mark.totalPart = numPart;
	pHead->mark.part = 0;
	//передача
	int sizePayOfCurPack = 0;
	while(pHead->mark.part < numPart) {
		if(pHead->mark.part == numPart - 1)
			sizePayOfCurPack = size;
		else
			sizePayOfCurPack = MAX_SIZE_PAYLOAD_PACK;
		size -= sizePayOfCurPack;
		std::copy_n(pTransBuf + pHead->mark.part * MAX_SIZE_PAYLOAD_PACK, sizePayOfCurPack, pData);
		mutTrans_.lock();
		sendto(sock_, &buf[0], sizePayOfCurPack + sizeof(mad_n::HeadPack), 0, reinterpret_cast<sockaddr*>(&addrMad_), sizeof(addrMad_));
		mutTrans_.unlock();
		pHead->mark.part++;
	}
}

void Detector::open_alg(const string& alg) const {
	int buf[2] = { OPEN_ALG };
	for (auto& p : alg_) {
		if (p.first == alg) {
			buf[1] = p.second;
			transCom(&buf, sizeof(buf));
			return;
		}
	}
	cout << "Алгоритм " << alg << " не поддерживается МАД\n";
	return;
}

void Detector::close_alg(const std::string& alg) const {
	int buf[2] = { CLOSE_ALG };
	for (auto& p : alg_) {
		if (p.first == alg) {
			buf[1] = p.second;
			transCom(&buf, sizeof(buf));
			return;
		}
	}
	cout << "Алгоритм " << alg << " не поддерживается МАД\n";
	return;
}

void Detector::close_all_alg(void) const {
	int buf = CLOSE_ALL_ALG;
	transCom(&buf, sizeof(buf));
}

void Detector::set_parameter_block(const unsigned int& bef,
		const unsigned int& after) const {
	unsigned int buf[3] = { SET_PB, bef, after };
	transCom(&buf, sizeof(buf));
	return;
}

void Detector::get_parameter_block(void) const {
	int buf = GET_PB;
	transCom(&buf, sizeof(buf));
}

void Detector::set_sigma(const int & s) const {
	int buf[2] = { SET_SIGMA, s };
	transCom(&buf, sizeof(buf));
	return;
}

void Detector::sync(void) {
	using namespace boost::posix_time;
	static std::array<int8_t, sizeof(HeadPack) + sizeof(int)> buf;
	static mad_n::HeadPack* pHead = reinterpret_cast<HeadPack*>(&buf[0]);
	pHead->mark.totalPart = 1;
	pHead->mark.part = 0;
	pHead->endpoints.idSrc = idB_;
	pHead->endpoints.idBlock = COMMAND;
	int* pCom = reinterpret_cast<int*>(&buf[sizeof(HeadPack)]);
	*pCom = SYNC;

	mutTrans_.lock();
	pHead->mark.id = getIdPack();
	sendto(sock_, &buf[0], buf.size(), 0, reinterpret_cast<sockaddr*>(&addrBroad_), sizeof(addrBroad_));
	mutTrans_.unlock();
	std::cout << "Отправлен сигнал синхронизации МАД" << std::endl;
	ptime t = second_clock::local_time();
	set_sync_time(t);
}

void Detector::single_sync(void) {
	static std::array<int8_t, sizeof(HeadPack) + sizeof(int)> buf;
	static mad_n::HeadPack* pHead = reinterpret_cast<HeadPack*>(&buf[0]);
	pHead->mark.totalPart = 1;
	pHead->mark.part = 0;
	pHead->endpoints.idSrc = idB_;
	pHead->endpoints.idBlock = COMMAND;
	int* pCom = reinterpret_cast<int*>(&buf[sizeof(HeadPack)]);
	*pCom = SYNC;

	mutTrans_.lock();
	pHead->mark.id = getIdPack();
	sendto(sock_, &buf[0], buf.size(), 0, reinterpret_cast<sockaddr*>(&addrMad_), sizeof(addrMad_));
	mutTrans_.unlock();
	std::cout << "Отправлен сигнал синхронизации МАД " << idM_ << std::endl;
}

void Detector::set_broadcast_address(const sockaddr_in& addr) {
	addrBroad_ = addr;
	return;
}

void Detector::set_sock(const int& sock) {
	sock_ = sock;
	return;
}

void Detector::set_src_id(const int& id) {
	idB_ = id;
	return;
}

void Detector::set_sync_time(boost::posix_time::ptime time) {
	std::lock_guard<std::mutex> lk(mutSyncTime_);
	syncTime_ = time;
	return;
}

boost::posix_time::ptime Detector::get_sync_time(void) {
	std::lock_guard<std::mutex> lk(mutSyncTime_);
	return syncTime_;
}

void Detector::enable_write_data(void) {
	isEnableWriteData_ = true;
	std::cout << "Мад: " << idM_ << ": Разрешено производить запись данных\n";
	return;
}

void Detector::disable_write_data(void) {
	isEnableWriteData_ = false;
	std::cout << "Мад: " << idM_ << ": Запрещено производить запись данных\n";
	return;
}

void Detector::enable_view_monitor(void) {
	isEnableShowMonit_ = true;
	std::cout << "Мад " << idM_
			<< ": Разрешено показывать информацию мониторограмм на экране\n";
	return;
}

void Detector::disable_view_monitor(void) {
	isEnableShowMonit_ = false;
	std::cout << "Мад: " << idM_
			<< ": Запрещено показывать информацию мониторограмм на экране\n";
	return;
}

void Detector::automatic_generation_of_sync(void) {
	for (;;) {
		sleep(PeriodOfSync);
		sync();
	}
	return;
}

void Detector::start_automatic_generation_of_sync(void) {
	std::thread t(automatic_generation_of_sync);
	t.detach();
	return;
}

void Detector::start_gasik(void) {
	int buf = START_GASIK;
	transCom(&buf, sizeof(buf));
	return;
}

void Detector::stop_gasik(void) {
	int buf = STOP_GASIK;
	transCom(&buf, sizeof(buf));
	return;
}

void Detector::set_period_monitor(const unsigned & s) const {
	int buf[2] = { SET_PERIOD_MONITOR };
	buf[1] = s;
	transCom(buf, sizeof(buf));
	return;
}

void Detector::get_period_monitor(void) const {
	int buf = GET_PERIOD_MONITOR;
	transCom(&buf, sizeof(buf));
	return;
}

void Detector::get_set_algorithm(void) const {
	int buf = GET_AC;
	transCom(&buf, sizeof(buf));
	return;
}

void Detector::get_sigma(void) const {
	int buf = GET_SIGMA;
	transCom(&buf, sizeof(buf));
	return;
}

void Detector::set_period_monitoring_pga(unsigned char period) {
	int buf[2] = { SET_PERIOD_MONITOR_PGA, static_cast<int>(period)};
	transCom(&buf, sizeof(buf));
	return;
}

void Detector::view_oscillograph(void) {
	if(osc_.start())
		std::cout << "Запущен осциллограф для мада " << idM_ << std::endl;
	else
		std::cout << "Не удалось запустить осциллограф для мада " << idM_ << std::endl;
}

void Detector::close_oscillograph(void) {
	if(osc_.isActive()) {
		osc_.stop();
		std::cout << "Осциллограф для мада " << idM_ << " деактивирован" << std::endl;
	} else
		std::cout << "Осциллограф для мада " << idM_ << " не был открыт" << std::endl;
}

std::string Detector::get_sync_time_Str(void) {
	return getTimeStr(get_sync_time());
}

Detector::~Detector() {
}

string find_name_algorithm(const int& id, const std::map<string, int>& m) {
	string s;
	for (auto &x : m)
		if (x.second == id)
			s = x.first;
	return s;
}

void Detector::handlingData(PtrData pPack) {
	using namespace genfiles_n;
	std::shared_ptr<GenFiles> pGen;
	if(pPack->size() < sizeof(GenFiles::Head)) {
		std::cout << "Принят пакет, размер которого меньше размера структуры GenFiles::Head" << std::endl;
		return;
	}
	GenFiles::Head* pHead = reinterpret_cast<GenFiles::Head*>(pPack->data());
	switch(pHead->numAlg) {
	case id_data::MONITOR:
		handlingMonitor(pPack, id_data::MONITOR);
		break;
	case id_data::FILTER:
		handlingFilter(pPack, id_data::FILTER);
		break;
	case id_data::GASIK:
		handlingGasik(pPack, id_data::GASIK);
		break;
	case id_data::CONTINIOUS:
		handlingContinious(pPack);
		break;
	default:
		std::cout << "Принят некорректный пакет с идентификатором " << pHead->numAlg << std::endl;
	}
}

void Detector::handlingMonitor(PtrData pPack, int idStor) {
	using namespace genfiles_n;
	GenFiles::Head* pHead = reinterpret_cast<GenFiles::Head*>(pPack->data());
	if(pPack->size() < sizeof(GenFiles::Head) + sizeof(GenMonitorFile::Monitor))
		return;
	GenMonitorFile::Monitor* pMonit = reinterpret_cast<GenMonitorFile::Monitor*>(pPack->data() + sizeof(GenFiles::Head));
	if (isEnableWriteData_) {
		std::shared_ptr<GenFiles> pGen(new GenMonitorFile(*pHead, *pMonit));
		pStorManager_->save(idStor, pGen);
	}
	if (isEnableShowMonit_)
		monitortoScreen(*pHead, *pMonit);
}

void Detector::handlingFilter(PtrData pPack, int idStor) {
	using namespace genfiles_n;
	if (!isEnableWriteData_)
		return;
	GenFiles::Head* pHead = reinterpret_cast<GenFiles::Head*>(pPack->data());
	if(pPack->size() < sizeof(GenFiles::Head) + sizeof(GenFilterAlgFile::Params) + sizeof(DataAlgorithm))
		return;
	GenFilterAlgFile::Params* pParams = reinterpret_cast<GenFilterAlgFile::Params*>(pPack->data() +
			sizeof(GenFiles::Head));
	DataAlgorithm* pDataAlg = reinterpret_cast<DataAlgorithm*>(pPack->data() +
			sizeof(GenFiles::Head) + sizeof(GenFilterAlgFile::Params));
	std::string nameFile = get_sync_time_Str() + "_" + std::to_string(pDataAlg->numFirstCount);
	auto posData = pPack->begin() + (&pDataAlg->data - pPack->data());
	std::shared_ptr<GenFiles> pGen(new GenFilterAlgFile(*pHead, *pParams, std::make_shared<PtrData::element_type>(posData, pPack->end())));
	pGen->setFileName(nameFile);
	pStorManager_->save(idStor, pGen);
}

void Detector::handlingGasik(PtrData pPack, int idStor) {
	using namespace genfiles_n;
	if (!isEnableWriteData_)
		return;
	GenFiles::Head* pHead = reinterpret_cast<GenFiles::Head*>(pPack->data());
	if(pPack->size() < sizeof(GenFiles::Head) + sizeof(GenGasikFile::Params) + sizeof(DataAlgorithm))
		return;
	GenGasikFile::Params* pParams = reinterpret_cast<GenGasikFile::Params*>(pPack->data() +
			sizeof(GenFiles::Head));
	DataAlgorithm* pDataAlg = reinterpret_cast<DataAlgorithm*>(pPack->data() +
			sizeof(GenFiles::Head) + sizeof(GenGasikFile::Params));
	std::string nameFile = get_sync_time_Str() + "_" + std::to_string(pDataAlg->numFirstCount);
	auto posData = pPack->begin() + (&pDataAlg->data - pPack->data());
	std::shared_ptr<GenFiles> pGen(new GenGasikFile(*pHead, *pParams, std::make_shared<PtrData::element_type>(posData, pPack->end())));
	pGen->setFileName(nameFile);
	pStorManager_->save(idStor, pGen);
}

void Detector::handlingContinious(PtrData pPack) {
	using namespace genfiles_n;
	pPack->erase(pPack->begin(), pPack->begin() + sizeof(GenFiles::Head) + offsetof(DataAlgorithm, data));
	writeDataInOsc(pPack);
}

void Detector::monitortoScreen(const genfiles_n::GenFiles::Head& head,
		const genfiles_n::GenMonitorFile::Monitor& monit) const {
	cout << "\n\n" << getTimeStr(boost::posix_time::second_clock::local_time()) << " МАД " << idM_
			<< " принята мониторограмма\n";
	cout << "freq = " << head.freq << " Hz " << " g1 = " << head.gain[0] << " dB " << " g2 = "
			<< head.gain[1] << " dB " << " g3 = "  << head.gain[2] << " dB " << " g4 = " << head.gain[3] << " dB\n";
	cout << " mean1 = " << monit.mean[0] << " rms1 = " << monit.rms[0] << std::endl;
	cout << " mean2 = " << monit.mean[1] << " rms2 = " << monit.rms[1] << std::endl;
	cout << " mean3 = " << monit.mean[2] << " rms3 = " << monit.rms[2] << std::endl;
	cout << " mean4 = " << monit.mean[3] << " rms4 = " << monit.rms[3] << std::endl;
}

void Detector::writeDataInOsc(PtrData pData) {
	if(osc_.isActive()){
		std::array<uint16_t, 4> buf;
		uint16_t* pSampl = reinterpret_cast<uint16_t*>(pData->data());
		int numSampl = pData->size() / sizeof(uint16_t);
		for(int i = 0; i < numSampl; i+=4) {
			std::copy(pSampl + i, pSampl + i +4, buf.begin());
			osc_.write(buf);
		}
	}
}

void Detector::handlingInfo(PtrData pPack) {
	if (pPack->size() < sizeof(int))
		return;
	int id = *(reinterpret_cast<int*>(pPack->data()));
	switch (id) {
	case OVERLOAD:
		cout << "\n\n" << getTimeStr(boost::posix_time::second_clock::local_time()) << " МАД " << idM_
		<< " принято сообщение о перегрузке каналов. Коэффициенты усиления PGA сброшены в ноль\n";
		break;
	default:
		cout << "\n\n" << getTimeStr(boost::posix_time::second_clock::local_time()) << " МАД " << idM_
		<< " принято неизвестное сообщение\n";
		break;
	}
	return;
}

void Detector::handlingCommandsReply(PtrData pPack) {
	unsigned int len = pPack->size();
	if (len < sizeof(h_pack_ans))
		return;
	h_pack_ans* panswer = reinterpret_cast<h_pack_ans*>(pPack->data());
	switch (panswer->id) {
	case ID_START_ADC:
		if (len != sizeof(h_pack_ans))
			return;
		cout << "Мад " << idM_
				<< (panswer->status == NOT_OK ?
						" не смог запустить" : " запустил")
						<< " АЦП преобразование\n";
		break;
	case ID_STOP_ADC:
		if (len != sizeof(h_pack_ans))
			return;
		cout << "Мад " << idM_
				<< (panswer->status == NOT_OK ?
						" не смог остановить" : " остановил")
						<< " АЦП преобразование\n";
		break;
	case ID_START_TEST:
		if (len != sizeof(h_pack_ans))
			return;
		cout << "Мад " << idM_
				<< (panswer->status == NOT_OK ?
						" не смог запустить" : " запустил")
						<< " тестовое преобразование\n";
		break;
	case ID_STOP_TEST:
		if (len != sizeof(h_pack_ans))
			return;
		cout << "Мад " << idM_
				<< (panswer->status == NOT_OK ?
						" не смог остановить" : " остановил")
						<< " тестовое преобразование\n";
		break;
	case ID_SET_GAIN:
		if (len != sizeof(h_pack_ans))
			return;
		cout << "Мад " << idM_
				<< (panswer->status == NOT_OK ?
						" не смог установить" : " установил")
						<< " новые коэффициенты усиления\n";
		break;
	case OPEN_ALG:
		if (len != sizeof(h_pack_ans))
			return;
		cout << "Мад " << idM_
				<< (panswer->status == NOT_OK ?
						" не смог запустить" : " запустил")
						<< " новый алгоритм обработки данных\n";
		break;
	case CLOSE_ALG:
		if (len != sizeof(h_pack_ans))
			return;
		cout << "Мад " << idM_
				<< (panswer->status == NOT_OK ?
						" не смог остановить" : " остановил")
						<< " алгоритм обработки данных\n";
		break;
	case CLOSE_ALL_ALG:
		if (len != sizeof(h_pack_ans))
			return;
		cout << "Мад " << idM_
				<< (panswer->status == NOT_OK ? " не смог закрыть" : " закрыл")
				<< " все алгоритмы обработки данных\n";
		break;
	case SET_SIGMA:
		if (len != sizeof(h_pack_ans))
			return;
		cout << "Мад " << idM_
				<< (panswer->status == NOT_OK ?
						" не смог установить" : " установил")
						<< " новый коэффициент превышения шума\n";
		break;
	case SET_PERIOD_MONITOR_PGA:
		if (len != sizeof(h_pack_ans))
			return;
		cout << "Мад " << idM_
				<< (panswer->status == NOT_OK ?
						" не смог установить" : " установил")
						<< " новый период мониторинга pga\n";
		break;
	case SET_PB:
		if (len != sizeof(h_pack_ans))
			return;
		cout << "Мад " << idM_
				<< (panswer->status == NOT_OK ?
						" не смог установить" : " установил")
						<< " новые параметры блока данных, передаваемого на БЦ\n";
		break;
	case GET_PB:
		if (len != sizeof(h_pack_ans) + 2 * sizeof(int))
			return;
		if (panswer->status == NOT_OK)
			cout << "Мад " << idM_
			<< " не смог предоставить  параметры блока данных, передаваемого на БЦ\n";
		else {
			int* arg = reinterpret_cast<int*>(reinterpret_cast<char*>(pPack->data())
					+ sizeof(h_pack_ans));
			cout << "В Мад " << idM_
					<< " установлены параметры блока данных, передаваемого на БЦ before = "
					<< arg[0] << " after = " << arg[1] << std::endl;
		}
		break;
	case GET_AC: {
		if (len < sizeof(h_pack_ans))
			return;
		if (len == sizeof(h_pack_ans)) {
			cout << "В Мад " << idM_
					<< " сейчас не действует ни один алгоритм\n";
			return;
		}
		string algorithms;
		string a;
		int* arg = reinterpret_cast<int*>(reinterpret_cast<char*>(pPack->data())
				+ sizeof(h_pack_ans));
		for (unsigned i = 0; i < (len - sizeof(h_pack_ans)) / sizeof(int);
				i++) {
			a = find_name_algorithm(arg[i], alg_);
			if (a == "")
				a = "неизвестный алгоритм";
			algorithms += a + " ";
		}
		cout << "В Мад " << idM_ << " сейчас действуют следующие алгоритмы: "
				<< algorithms << std::endl;
	}
	break;
	case SYNC:
		if (len != sizeof(h_pack_ans))
			return;
		cout << "Мад " << idM_
				<< (panswer->status == NOT_OK ?
						" не смог осуществить" : " осуществил")
						<< " синхронизацию\n";
		break;
	case SET_PERIOD_MONITOR:
		if (len != sizeof(h_pack_ans))
			return;
		cout << "Мад " << idM_
				<< (panswer->status == NOT_OK ?
						" не смог установить" : " установил")
						<< " новое значение периода передачи мониторограмм\n";
		break;
	case GET_PERIOD_MONITOR:
		if (len != sizeof(h_pack_ans) + sizeof(int))
			return;
		if (panswer->status == NOT_OK)
			cout << "Мад " << idM_
			<< " не смог сообщить текущее значение периода передачи мониторограмм на БЦ\n";
		else {
			int* arg = reinterpret_cast<int*>(reinterpret_cast<char*>(pPack->data())
					+ sizeof(h_pack_ans));
			cout << "В Мад " << idM_
					<< " текущее значение периода передачи мониторограмм на БЦ равно "
					<< *arg << " секунд" << std::endl;
		}
		break;
	case GET_SIGMA:
		if (len != sizeof(h_pack_ans) + sizeof(int))
			return;
		if (panswer->status == NOT_OK)
			cout << "Мад " << idM_
			<< " не смог сообщить текущее значение коэффициента превышения порога шума\n";
		else {
			int* arg = reinterpret_cast<int*>(reinterpret_cast<char*>(pPack->data())
					+ sizeof(h_pack_ans));
			cout << "В Мад " << idM_
					<< " текущее значение коэффициента превышения порога шума равно "
					<< *arg << std::endl;
		}
		break;
	}
	return;
}

void Detector::receive(PtrData pPack) {
	PtrData pBuf;
	int idBlock = -1;
	std::make_pair(std::ref(pBuf), std::ref(idBlock)) = collector_.receivBlock(pPack);
	if (pBuf == nullptr)
		return;
	switch(idBlock) {
	case ANSWER:
#ifdef PRINT_INFO_REC_PACK
		std::cout << "Принят пакет, содержащий ответ на команду" << std::endl;
#endif //PRINT_INFO_REC_PACK
		handlingCommandsReply(pBuf);
		break;
	case DATA:
#ifdef PRINT_INFO_REC_PACK
		std::cout << "Принят пакет данных алгоритма выделения" << std::endl;
#endif //PRINT_INFO_REC_PACK
		handlingData(pBuf);
		break;
	case INFO:
#ifdef PRINT_INFO_REC_PACK
		std::cout << "Принят информационный пакет" << std::endl;
#endif //PRINT_INFO_REC_PACK
		handlingInfo(pBuf);
		break;
	default:
#ifdef PRINT_INFO_REC_PACK
		std::cout << "Принятый пакет данных содержит неизвестный идентификатор типа пакета" << std::endl;
#endif //PRINT_INFO_REC_PACK
		break;
	}
}

} /* namespace mad_n */
