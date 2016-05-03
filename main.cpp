/*
 * main.cpp
 *
 *  Created on: 22 февр. 2014 г.
 *      Author: andrej
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include "Detector.h"
#include <map>
#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <exception>
#include "Test.h"
#include "Gasik.h"

using mad_n::Detector;
using namespace std;

#define MAD_ADDR1	"192.168.203.31"
#define MAD_ADDR2	"192.168.203.32"
#define MAD_ADDR3	"192.168.203.33"
#define MAD_ADDR4	"192.168.203.34"
#define BROADCAST_ADDR "192.168.100.255"
#define BAG_PORT	30000
#define MAD_PORT	30000
#define MAX_SIZE_SAMPL_REC 4100 //определяет длину буфера передачи сокета
#define ID_SC 4
#define PERIOD_OF_SYNC 300

//коммуникация с компьютером Гасик
#define GASIK_PC_ADDR	"192,168.1.155"
#define GASIK_PC_PORT 31001

void start_automatic_generation_of_sync(std::map<unsigned int, Detector*>& det);	//старт задачи, отвечающей за синхронизацию МАД в системе  путём поочередной посылки синхросигнала
void automatic_generation_of_sync(std::map<unsigned int, Detector*>& det);
void hand_command_line(std::map<unsigned int, Detector*>& det); //обработка командной строки

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "Вызов программы должен сопровождаться хотя бы одним параметром\n";
		return 1;
	}
	int sock;
	bool stdinIsEnable = true;
	if(argc >= 3 && strcmp(argv[2],"-d") == 0) //если среди параметров присутствует ключ, запрещающий приём данных из стандартного ввода
		stdinIsEnable = false;
	sockaddr_in addrBag;
	std::map<unsigned int, Detector*> detSet;

	bzero(&addrBag, sizeof(addrBag));
	addrBag.sin_family = AF_INET;
	addrBag.sin_port = htons(BAG_PORT);
	addrBag.sin_addr.s_addr = htonl(INADDR_ANY);
	//создание сокета
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		std::cerr << "создать сокет не удалось\n";
		exit(1);
	}
	if (bind(sock, reinterpret_cast<sockaddr*>(&addrBag), sizeof(addrBag))) {
		std::cerr << "не удалось связать сокет с адресом\n";
		exit(1);
	}
	int sizeRec = MAX_SIZE_SAMPL_REC * 4 * sizeof(short);
	if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &sizeRec, sizeof(int)) == -1) {
		std::cerr << "Не поддерживается объём буфера приёма сокета в размере "
				<< sizeRec << " байт\n";
		exit(1);
	}
	int tm = 1;
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*) &tm, sizeof(tm));
	//создание  сокета приёма инструкций от компьютера ГАСИК
	int sockGas;
	sockaddr_in addrGas;
	bzero(&addrGas, sizeof(addrGas));
	addrGas.sin_family = AF_INET;
	addrGas.sin_port = htons(GASIK_PC_PORT);
	addrGas.sin_addr.s_addr = htonl(INADDR_ANY);
	sockGas = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockGas == -1) {
		std::cerr
		<< "создать сокет приёма данных от компьютера Гасик не удалось\n";
		exit(1);
	}
	if (bind(sockGas, reinterpret_cast<sockaddr*>(&addrGas), sizeof(addrGas))) {
		std::cerr
		<< "не удалось связать сокет приёма данных от компьютера Гасик с адресом\n";
		exit(1);
	}
	//создание широковещательного адреса
	sockaddr_in broadAddr;
	bzero(&broadAddr, sizeof(broadAddr));
	broadAddr.sin_family = AF_INET;
	broadAddr.sin_port = htons(MAD_PORT);
	inet_pton(AF_INET, BROADCAST_ADDR, &broadAddr.sin_addr);
	Detector::set_broadcast_address(broadAddr);
	Detector::set_sock(sock);
	Detector::set_src_id(ID_SC);

	//создание менеджера хранилищ
	std::shared_ptr<genfiles_n::StorageManager> pStorageManager(new genfiles_n::StorageManager(argv[1]));
	pStorageManager->addStorage(Detector::MONITOR, 3, "monitor", true);
	pStorageManager->addStorage(Detector::FILTER, 3, "filter", false);
	pStorageManager->addStorage(Detector::GASIK, 3, "gasik", false);
	//инициализация набора МАД
	detSet.insert(std::make_pair(1, new Detector(MAD_ADDR1, MAD_PORT, 1, pStorageManager)));
	detSet.insert(std::make_pair(2, new Detector(MAD_ADDR2, MAD_PORT, 2, pStorageManager)));
	detSet.insert(std::make_pair(3, new Detector(MAD_ADDR3, MAD_PORT, 3, pStorageManager)));
	detSet.insert(std::make_pair(4, new Detector(MAD_ADDR4, MAD_PORT, 4, pStorageManager)));
	//Detector::start_automatic_generation_of_sync();
	start_automatic_generation_of_sync(detSet);
	//создание объекта управления Гасиком
	gasik_n::Gasik gasik(GASIK_PC_ADDR, GASIK_PC_PORT, sockGas);
	for(auto& det : detSet) {
		gasik.addCallFuncAtStart(std::bind(&Detector::start_gasik, det.second));
		gasik.addCallFuncAtStop(std::bind(&Detector::stop_gasik, det.second));
	}
	//тестирование
#ifdef _DEBUG
	test_n::test();
#endif //_DEBUG
	fd_set fdin; //набор дескрипторов, на которых ожидаются входные данные
	int status = 0;

	for (;;) {
		FD_ZERO(&fdin);
		if(stdinIsEnable)
			FD_SET(STDIN_FILENO, &fdin);
		FD_SET(sock, &fdin);
		FD_SET(sockGas, &fdin);
		//ожидание событий
		status = select(sockGas + 1, &fdin, NULL, NULL, NULL);
		if (status == -1) {
			if (errno == EINTR)
				continue;
			else {
				perror("Функция select завершилась крахом\n");
				exit(1);
			}
		}
		if (FD_ISSET(STDIN_FILENO, &fdin))
			hand_command_line(detSet);
		if (FD_ISSET(sock, &fdin)) {
			sockaddr_in srcAddr;
			unsigned int sizeAddr = sizeof(srcAddr), sizeBuf = 0;
			if ( ioctl (sock,FIONREAD,&sizeBuf) < 0 )
			{
				perror("Применение ioctl для получения информации о размере очередной датаграмы завершилось неудачей\n");
				exit(1);
			}
			PtrData pBuf(new PtrData::element_type(sizeBuf));
			unsigned len = recvfrom(sock, reinterpret_cast<void *>(pBuf->data()), pBuf->size(), 0,
					reinterpret_cast<sockaddr*>(&srcAddr), &sizeAddr);
			if (len > sizeof(mad_n::HeadPack)) {
				mad_n::HeadPack* pHead = reinterpret_cast<mad_n::HeadPack*>(pBuf->data());
				if (detSet.count(pHead->endpoints.idSrc) != 0) {
					detSet[pHead->endpoints.idSrc]->receive(pBuf);
				} else {
					std::cout << "Получен пакет с некорректным идентификатором источника" << std::endl;
				}
			} else {
				std::cout << "Получен пакет, размер которого меньше размера структуры mad_n::HeadPack" << std::endl;
			}
		}
		if (FD_ISSET(sockGas, &fdin)) {
			sockaddr_in srcAddr;
			unsigned int size = sizeof(srcAddr), sizeBuf = 0;
			if ( ioctl (sockGas,FIONREAD,&sizeBuf) < 0 )
			{
				perror("Применение ioctl для получения информации о размере очередной датаграмы завершилось неудачой\n");
				exit(1);
			}
			PtrData pBuf(new PtrData::element_type(sizeBuf));
			recvfrom(sockGas, reinterpret_cast<void *>(pBuf->data()), pBuf->size(),
					0, reinterpret_cast<sockaddr*>(&srcAddr), &size);
			gasik.receivMessage(pBuf);
		}
	}
	return 0;
}

void start_automatic_generation_of_sync(std::map<unsigned int, Detector*>& det) {
	std::thread t(automatic_generation_of_sync, std::ref(det));
	t.detach();
	return;
}

void automatic_generation_of_sync(std::map<unsigned int, Detector*>& det) {
	using namespace boost::posix_time;
	for (;;) {
		for (auto& d : det)
			d.second->single_sync();
		ptime t = second_clock::local_time();
		Detector::set_sync_time(t);
		sleep(PERIOD_OF_SYNC);
	}
	return;
}

void hand_command_line(std::map<unsigned int, Detector*>& det) {
	std::istringstream message;
	string command, mes;
	int id;
	try {
		getline(cin, command);
		message.str(command);
		message >> mes;
		if (mes == "mad1")
			id = 1;
		else if (mes == "mad2")
			id = 2;
		else if (mes == "mad3")
			id = 3;
		else if (mes == "mad4")
			id = 4;
		else if (mes == "mads")
			id = -1;
		else if (mes == "exit")
			exit(0);
		else {
			cout << "Пункта назначения " << mes << " не существует" << endl;
			return;
		}
		message >> mes;
		if (mes == "start") {
			if (id == -1)
				for (auto& d : det)
					d.second->start_adc();
			else
				det[id]->start_adc();
		} else if (mes == "stop") {
			if (id == -1)
				for (auto& d : det)
					d.second->stop_adc();
			else
				det[id]->stop_adc();
		} else if (mes == "startt") {
			if (id == -1)
				for (auto& d : det)
					d.second->start_test();
			else
				det[id]->start_test();
		} else if (mes == "stopt") {
			if (id == -1)
				for (auto& d : det)
					d.second->stop_test();
			else
				det[id]->stop_test();
		} else if (mes == "gain") {
			int buf[4];
			for (int i = 0; i < 4; i++) {
				message >> mes;
				buf[i] = stoi(mes);
			}
			if (id == -1)
				for (auto& d : det)
					d.second->set_gain(buf);
			else
				det[id]->set_gain(buf);
		} else if (mes == "open_a") {
			message >> mes;
			if (id == -1)
				for (auto& d : det)
					d.second->open_alg(mes);
			else
				det[id]->open_alg(mes);
		} else if (mes == "close_a") {
			message >> mes;
			if (id == -1)
				for (auto& d : det)
					d.second->close_alg(mes);
			else
				det[id]->close_alg(mes);
		} else if (mes == "close_aa") {
			if (id == -1)
				for (auto& d : det)
					d.second->close_all_alg();
			else
				det[id]->close_all_alg();
		} else if (mes == "set_sigma") {
			message >> mes;
			int num = stoi(mes);
			if (id == -1)
				for (auto& d : det)
					d.second->set_sigma(num);
			else
				det[id]->set_sigma(num);
		} else if (mes == "set_p_pm") {
			message >> mes;
			unsigned char period = static_cast<unsigned char>(stoi(mes));
			if (id == -1)
				for (auto& d : det)
					d.second->set_period_monitoring_pga(period);
			else
				det[id]->set_period_monitoring_pga(period);
		} else if (mes == "set_par_b") {
			message >> mes;
			unsigned int bef = stoi(mes);
			message >> mes;
			unsigned int aft = stoi(mes);
			if (id == -1)
				for (auto& d : det)
					d.second->set_parameter_block(bef, aft);
			else
				det[id]->set_parameter_block(bef, aft);
		} else if (mes == "get_par_b") {
			if (id == -1)
				for (auto& d : det)
					d.second->get_parameter_block();
			else
				det[id]->get_parameter_block();
		} else if (mes == "sync") {
			if (id == -1)
				Detector::sync();
			else
				det[id]->single_sync();
		}
		else if (mes == "e_wd") {
			if (id == -1)
				for (auto& d : det)
					d.second->enable_write_data();
			else
				det[id]->enable_write_data();
		} else if (mes == "d_wd") {
			if (id == -1)
				for (auto& d : det)
					d.second->disable_write_data();
			else
				det[id]->disable_write_data();
		} else if (mes == "e_dm") {
			if (id == -1)
				for (auto& d : det)
					d.second->enable_view_monitor();
			else
				det[id]->enable_view_monitor();
		} else if (mes == "d_dm") {
			if (id == -1)
				for (auto& d : det)
					d.second->disable_view_monitor();
			else
				det[id]->disable_view_monitor();
		} else if (mes == "set_p_dm") {
			message >> mes;
			int num = stoi(mes);
			if (id == -1)
				for (auto& d : det)
					d.second->set_period_monitor(num);
			else
				det[id]->set_period_monitor(num);
		} else if (mes == "get_p_dm")
			if (id == -1)
				for (auto& d : det)
					d.second->get_period_monitor();
			else
				det[id]->get_period_monitor();
		else if (mes == "get_ac")
			if (id == -1)
				for (auto& d : det)
					d.second->get_set_algorithm();
			else
				det[id]->get_set_algorithm();
		else if (mes == "get_sigma")
			if (id == -1)
				for (auto& d : det)
					d.second->get_sigma();
			else
				det[id]->get_sigma();
		else if (mes == "v_osc")
			if (id == -1)
				for (auto& d : det)
					d.second->view_oscillograph();
			else
				det[id]->view_oscillograph();
		else if (mes == "d_osc")
			if (id == -1)
				for (auto& d : det)
					d.second->close_oscillograph();
			else
				det[id]->close_oscillograph();
		else
			cout << "Передана неизвестная команда\n";
	} catch (...) {
		std::cout << "Неверный формат команды\n";
	}
	return;
}
