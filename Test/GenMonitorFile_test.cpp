/*
 * GenMonitorFile_test.cpp
 *
 *  Created on: 15 апр. 2015 г.
 *      Author: andrej
 */

#include "GenMonitorFile_test.h"
#include <assert.h>
#include <boost/filesystem.hpp>

namespace genfiles_n {

GenMonitorFile::Monitor TestMonitGenFiles::getMonitor() {
	GenMonitorFile::Monitor monitor;
	for(int i = 0; i < 4; i++) {
		monitor.mean[i] = 2050;
		monitor.rms[i] = 2100 + i * 100;
	}
	return monitor;
}

GenMonitorFile::Monitor TestMonitGenFiles::getOtherMonitor() {
	GenMonitorFile::Monitor monitor = getMonitor();
	for(int i = 0; i < 4; i++)
		monitor.mean[i] = monitor.mean[i] << 1;
	return monitor;
}

void TestMonitGenFiles::test(void) {
	using namespace boost::filesystem;
	GenFiles::Head head = TestGenFiles(1).getHead(1);
	GenMonitorFile gen(head, getMonitor());
	path p(TEST_DIRECTORY_);
	assert(is_directory(p));
	p /= gen.getMetaFileName();
	assert(gen.fillInMetaFile(p.string()));
	p = TEST_DIRECTORY_;
	p /= gen.getFileName();
	assert(gen.fillHeadlineInFile(p.string()));
	assert(gen.fillInFile(p.string()));
	assert(gen.fillInFile(p.string()));
	assert(gen.fillInFile(p.string()));
}

GenMonitorFile TestMonitGenFiles::getGen(int numMad) {
	GenFiles::Head head = TestGenFiles(numMad).getHead(numMad);
	return GenMonitorFile(head, getMonitor());
}

GenMonitorFile TestMonitGenFiles::getOtherGen(int numMad) {
	GenFiles::Head head = TestGenFiles(numMad).getHead(numMad);
	head.freq <<= 1;
	return GenMonitorFile(head, getMonitor());
}

TestMonitGenFiles::TestMonitGenFiles(const std::string& dir): TEST_DIRECTORY_(dir) {
}

} /* namespace genfiles_n */
