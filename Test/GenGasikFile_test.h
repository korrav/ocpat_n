/*
 * GenGasikFile_test.h
 *
 *  Created on: 22 апр. 2015 г.
 *      Author: andrej
 */

#ifndef GENGASIKFILE_TEST_H_
#define GENGASIKFILE_TEST_H_
#include "GenGasikFile.h"

namespace genfiles_n {
class TestGasikGenFiles {
	const std::string TEST_DIRECTORY_;
	void testCompareMeta(void);
	void testCreateFiles(void);
public:
	static PtrData getBuffer();
	void test(void);
	TestGasikGenFiles(const std::string& dir = "/tmp");
};
}
#endif /* GENGASIKFILE_TEST_H_ */
