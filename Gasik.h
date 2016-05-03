/*
 * Gasik.h
 *
 *  Created on: 26 мая 2015 г.
 *      Author: andrej
 */

#ifndef GASIK_H_
#define GASIK_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <mutex>
#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include "GeneratorFiles.h"

namespace gasik_n {

class Gasik {
	int sock_;
	sockaddr_in addrPcGasik_;
	std::vector<std::function<void(void)>> funcsAtStart_;	//содержит функции, которые вызываются при старте режима Гасик
	std::vector<std::function<void(void)>> funcsAtStop_;	//содержит функции, которые вызываются при остановке режима Гасик
	std::mutex mut_;
	boost::asio::io_service ioservice;
	boost::asio::steady_timer timer_{ioservice};
	bool is_start_gasik_ = false;
	constexpr static int COM_START = 1; //инструкция запуска сеанса Гасик
	constexpr static int COM_STOP = 0; //инструкция остановки сеанса Гасик
	const int DuratinSeanseGasik = 1200; //продолжительность сеанса Гасик (сек)

	void start(void);	//запуск режима Гасик
	void stop(void);	//остановка режима Гасик
public:
	void addCallFuncAtStart(std::function<void(void)> callF);	//добавление функций, которые вызывается при запуске сеанса ГАСИК
	void addCallFuncAtStop(std::function<void(void)> callF);	//добавление функций, которые вызывается при остановке сеанса ГАСИК
	void receivMessage(PtrData pBuf);	//приём сообщения
	Gasik(const char* ip, unsigned short port, int sock);
	virtual ~Gasik();
};

} /* namespace gasik_n */

#endif /* GASIK_H_ */
