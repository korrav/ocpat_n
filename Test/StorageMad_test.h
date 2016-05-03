/*
 * Storage_test.h
 *
 *  Created on: 28 апр. 2015 г.
 *      Author: andrej
 */

#ifndef STORAGEMAD_TEST_H_
#define STORAGEMAD_TEST_H_
#include <StorageMad.h>

namespace genfiles_n {
class TestContentMadsDir {
	const std::string TEST_DIRECTORY_ = "/tmp";
	const int NUMBER_GENERATE_FILES = 100;
public:
	TestContentMadsDir(const std::string& dir = "/tmp");
	void test(void);
	~TestContentMadsDir();
private:
	StorageMad* storSingleFileInDir_ = nullptr;
	StorageMad* storNumberFileInDir_ = nullptr;
};

class TestStorage {
	const std::string TEST_DIRECTORY_ = "/tmp";
	const int NUMBER_GENERATE_FILES = 100;
	enum { MONITOR, FILTER, INVALID};
public:
	TestStorage(const std::string& dir = "/tmp");
	void test(void);
	~TestStorage();
};

}

#endif /* STORAGEMAD_TEST_H_ */
