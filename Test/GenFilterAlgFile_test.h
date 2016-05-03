/*
 * GenFilterAlgFile_test.h
 *
 *  Created on: 18 апр. 2015 г.
 *      Author: andrej
 */

#ifndef GENFILTERALGFILE_TEST_H_
#define GENFILTERALGFILE_TEST_H_
#include "GenFilterAlgFile.h"

namespace genfiles_n {
class TestFilterAlgGenFiles {
	const std::string TEST_DIRECTORY_ = "/tmp";
	void testCompareMeta(void);
	void testCreateFiles(void);
public:
	static PtrData getBuffer();
	static GenFilterAlgFile getGen(int numMad);
	static GenFilterAlgFile getOtherGen(int numMad);
	void test(void);
	TestFilterAlgGenFiles(const std::string& dir = "/tmp");
};
}
#endif /* GENFILTERALGFILE_TEST_H_ */
