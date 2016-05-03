/*
 * GenFiles_test.h
 *
 *  Created on: 13 апр. 2015 г.
 *      Author: andrej
 */

#ifndef GENFILES_TEST_H_
#define GENFILES_TEST_H_
#include "GeneratorFiles.h"
#include <map>

namespace genfiles_n {
class TestGenFiles {
	const std::string TEST_DIRECTORY_ = "/tmp";
public:
	typedef GenFiles::Head Head;
	TestGenFiles(unsigned numMads, const std::string& dir = "/tmp");
	Head getHead(int mad);
	Head getDifferentHead(int mad);
	void test(void);
	~TestGenFiles();
private:
	std::map<int, Head> headMads_;
	Head buildHead(int mad);
	void testCompareHead(void);
	void testCreateMetaFile(const std::string& dir);
};

} /* namespace collector_n */
#endif /* GENFILES_TEST_H_ */

