/*
 * Storage_test.cpp
 *
 *  Created on: 23 апр. 2015 г.
 *      Author: andrej
 */

#include <boost/filesystem.hpp>
#include <StorageMad_test.h>
#include <random>
#include <time.h>
#include <thread>
#include <chrono>
#include "GenMonitorFile_test.h"
#include "GenFilterAlgFile_test.h"
#include <memory>

using namespace boost::filesystem;
namespace genfiles_n {
TestContentMadsDir::TestContentMadsDir(const std::string& dir): TEST_DIRECTORY_(dir){
	path patSingle = dir + "/single";
	if(!is_directory(patSingle))
		assert(create_directories(patSingle));
	storSingleFileInDir_ = new StorageMad(patSingle.string(), true);
	path patMulti = dir + "/multi";
	if(!is_directory(patMulti))
		assert(create_directories(patMulti));
	storNumberFileInDir_ = new StorageMad(patMulti.string(), false);
	std::cout << "Созданы папки " << patSingle << " и " << patMulti << std::endl;
}

void TestContentMadsDir::test(void) {
	typedef GenMonitorFile(*funcCreateGenMonit)(int);
	typedef GenFilterAlgFile(*funcCreateGenFilter)(int);
	std::shared_ptr<GenFiles> pGen;
	std::default_random_engine dre(time(nullptr));
	std::array<funcCreateGenMonit, 2> createGenMonit = {TestMonitGenFiles::getGen, TestMonitGenFiles::getOtherGen};
	std::array<funcCreateGenFilter, 2> createGenFilter = {TestFilterAlgGenFiles::getGen, TestFilterAlgGenFiles::getOtherGen};
	std::discrete_distribution<int> di{9, 1};
	for(int i = 0; i < NUMBER_GENERATE_FILES; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		pGen.reset(new GenMonitorFile(createGenMonit[di(dre)](1)));
		storSingleFileInDir_->save(pGen);
		pGen.reset(new GenFilterAlgFile(createGenFilter[di(dre)](1)));
		storNumberFileInDir_->save(pGen);
	}
}

TestContentMadsDir::~TestContentMadsDir() {
	std::cout << "Задача выполнена\n";
	delete storSingleFileInDir_;
	storSingleFileInDir_ = nullptr;
	delete storNumberFileInDir_;
	storNumberFileInDir_ = nullptr;
}

TestStorage::TestStorage(const std::string& dir): TEST_DIRECTORY_(dir) {
}

void TestStorage::test(void) {
	typedef GenMonitorFile(*funcCreateGenMonit)(int);
	typedef GenFilterAlgFile(*funcCreateGenFilter)(int);
	path patDir = TEST_DIRECTORY_;
	if(!is_directory(patDir))
		assert(create_directories(patDir));
	StorageManager manager(patDir.string());
	manager.addStorage(MONITOR, 4, "monitor", true);
	manager.addStorage(FILTER, 4, "filter", false);

	std::shared_ptr<GenFiles> pGen;
	std::default_random_engine dre(time(nullptr));
	std::array<funcCreateGenMonit, 2> createGenMonit = {TestMonitGenFiles::getGen, TestMonitGenFiles::getOtherGen};
	std::array<funcCreateGenFilter, 2> createGenFilter = {TestFilterAlgGenFiles::getGen, TestFilterAlgGenFiles::getOtherGen};
	std::discrete_distribution<int> di{9, 1};
	for(int i = 0; i < NUMBER_GENERATE_FILES; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		pGen.reset(new GenMonitorFile(createGenMonit[di(dre)](1)));
		manager.save(MONITOR, pGen);
		pGen.reset(new GenFilterAlgFile(createGenFilter[di(dre)](1)));
		if(i == 97)
			manager.save(INVALID, pGen);
		else
			manager.save(FILTER, pGen);
		if(i == 33)
			manager.addStorage(MONITOR, 4, "monitor1", true);
		if(i == 88)
			manager.addStorage(FILTER, 4, "filter1", false);
	}

}

TestStorage::~TestStorage() {
}

} /* namespace genfiles_n */
