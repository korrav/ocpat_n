/*
 * Storage.h
 *
 *  Created on: 23 апр. 2015 г.
 *      Author: andrej
 */

#ifndef STORAGEMAD_H_
#define STORAGEMAD_H_
#include "GeneratorFiles.h"
#include <boost/filesystem.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <type_traits>
#include <memory>
#include <map>
#include <future>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iostream>

namespace genfiles_n {
using namespace boost::filesystem;
using namespace boost::gregorian;

template <typename Func>
class ManagerThread;

template <typename R, typename ...Args>
class ManagerThread<R(Args...)> {
	using Result = R;
	using Func = R(Args...);
	using Future = std::future<Result>;
	boost::ptr_list<Future> fifoThreads_;	//очередь будущих результатов потоков
public:
	void run(Func fun, Args... args) {	//запускает выполнение переданной задачи
		fifoThreads_.push_back(new Future(std::async(std::launch::async, fun, args...)));
		fifoThreads_.erase_if([](const Future& fut) {return fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready;});
	}
	void waitForExec(void) {	//ожидание выполнения всех запущенных задач
		for(auto& job : fifoThreads_)
			job.get();
		fifoThreads_.clear();
	}
};

void archive(path dir, std::string strSuf);


class StorageMad {  //класс, управляющий сохранением данных для конкретного МАД
	path root_; //корневая директория (например, ....../mad1/)
	path curDir_;   //текущая директория, куда сохраняются данные МАДа
	date curDay_;    //дата текущего дня
	int curArchive_ = 0;    //директории, в которых сохраняются данные, представляют собой порядковые номера.Здесь текущий номер
	int numFile_ = 0;
	const std::string SUFFIX_FOR_ARCHIVE = ".zip";  //расширение файлов данных
	const int MAX_NUMBER_FILES_IN_DIR = 1000;   //максимальное количество данных в одном архиве
	std::shared_ptr<GenFiles> pPrevFile_ = nullptr; //указатель на предыдущий принятый генератор файлов данных
	bool isSingleFileInArchDir_;	/*Если true, то в одной архивной папке содержится только один файл,
		который при получении очередного генератора дописывается*/

	std::string makeDirCurDate();   //создаёт директорию для сохранения данных и возвращает её имя
	void makeArchive(const path& nameDirForArchive); //архивирование директории данных
	void makeArchiveAndCdNewDirectory(std::shared_ptr<GenFiles> pFileForMeta);	//создать архив и перейти в новую директорию
	int getNumberLastArchiveOfDir(const path& dir) const;   //получить наивысший номер архива данных, присутствующего в директории
	bool createNewDirCountAndCd(path& parent);  //создать новую папку данных и перейти туда
	bool isEqualityMetaData(std::shared_ptr<GenFiles> pFile1, std::shared_ptr<GenFiles> pFile2) const;  //проверка на равенство метаданных
public:
	StorageMad(std::string rootDir, bool isSingleFileInArchDir = false);
	~StorageMad();
	void save(std::shared_ptr<GenFiles> pFile);    //создание файла данных на основе генератора файлов и его сохранение
private:
	ManagerThread<decltype(archive)> manager_;
};

class Storage {	//хранилище файлов данных
	boost::filesystem::path root_;	//корневая папка хранилища
	std::map<int, StorageMad> poolMads_; //пул хранилищ МАДов
public:
	Storage(const std::string& dir, bool isSingleFileInArchDir_, int numMads);
	void save(std::shared_ptr<GenFiles> pFile);   //сохранение файла в хранилище
};

class StorageManager {	//менеджер хранилищ файлов данных
	boost::filesystem::path root_;	//корневая папка хранилища
	std::map<int, Storage> pool_; //пул хранилищ
public:
	StorageManager(std::string dirName);
	void addStorage(int idStorage, int numMads, const std::string& dir, bool isSingleFileInArchDir);	//добавить хранилище в пул
	void save(int idStorage, std::shared_ptr<GenFiles> pFile);   //сохранение файла в хранилище
	virtual ~StorageManager();
};

} /* namespace genfiles_n */

#endif /* STORAGEMAD_H_ */
