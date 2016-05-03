/*
 * GenMonitorFile_test.h
 *
 *  Created on: 17 апр. 2015 г.
 *      Author: andrej
 */

#ifndef GENMONITORFILE_TEST_H_
#define GENMONITORFILE_TEST_H_
#include "GenFiles_test.h"
#include "GenMonitorFile.h"

namespace genfiles_n {
class TestMonitGenFiles {
	const std::string TEST_DIRECTORY_;
public:
	static GenMonitorFile::Monitor getMonitor();
	static GenMonitorFile::Monitor getOtherMonitor();
	static GenMonitorFile getGen(int numMad);
	static GenMonitorFile getOtherGen(int numMad);
	void test(void);
	TestMonitGenFiles(const std::string& dir = "/tmp");
};

} /* namespace genfiles_n */

#endif /* GENMONITORFILE_TEST_H_ */
