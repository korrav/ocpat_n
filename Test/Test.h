/*
 * Test.h
 *
 *  Created on: 15 апр. 2015 г.
 *      Author: andrej
 */

#ifndef TEST_H_
#define TEST_H_
#include "GenFiles_test.h"
#include "GenMonitorFile_test.h"
#include "GenFilterAlgFile_test.h"
#include "GenGasikFile_test.h"
#include "Collector_test.h"
#include <boost/filesystem.hpp>
#include <assert.h>
#include <StorageMad_test.h>
#include <boost/date_time/posix_time/posix_time.hpp>

//ТЕСТЫ
//#define TEST_GENERATOR_FILES
#define TEST_COLLECTOR
//ТЕСТЫ




namespace test_n {
inline std::string getCurrentTimeStr(void) {
	using namespace boost::posix_time;
	ptime t = microsec_clock::local_time();
	std::stringstream ss;
	time_facet* pFacet = new time_facet("%d%m%y_%H%M%s");
	ss.imbue(std::locale{ss.getloc(), pFacet});
	ss << t;
	return ss.str();
}

inline void test_gen(bool withErase = true) {
	using namespace boost::filesystem;
	std::string dirRoot = "/tmp/test_generate";
	std::string dirGenerate = "generate";
	std::string dirMonit = "monit";
	std::string dirFilter = "filter";
	std::string dirGasik = "gasik";
	std::string dirContentMadsDir = "contentmadsdir";
	std::string dirStorage = "storage";
	path patRoot(dirRoot);
	if(!is_directory(patRoot))
		assert(create_directories(patRoot));

	path patGenerate = patRoot / dirGenerate;
	if(is_directory(patGenerate))
		remove_all(patGenerate);
	assert(create_directories(patGenerate));
	genfiles_n::TestGenFiles testGen(4, patGenerate.string());
	testGen.test();

	path patMonit = patRoot / dirMonit;
	if(is_directory(patMonit))
		remove_all(patMonit);
	assert(create_directories(patMonit));
	genfiles_n::TestMonitGenFiles(patMonit.string()).test();

	path patFilter = patRoot / dirFilter;
	if(is_directory(patFilter))
		remove_all(patFilter);
	assert(create_directories(patFilter));
	genfiles_n::TestFilterAlgGenFiles(patFilter.string()).test();

	path patGasik = patRoot / dirGasik;
	if(is_directory(patGasik))
		remove_all(patGasik);
	assert(create_directories(patGasik));
	genfiles_n::TestGasikGenFiles(patGasik.string()).test();

	path patContentMadsDir = patRoot / dirContentMadsDir;
	if(!is_directory(patContentMadsDir))
		assert(create_directories(patContentMadsDir));
	genfiles_n::TestContentMadsDir(patContentMadsDir.string()).test();

	path patStorage = patRoot / dirStorage;
	if(!is_directory(patStorage))
		assert(create_directories(patStorage));
	genfiles_n::TestStorage(patStorage.string()).test();

	if(withErase)
		remove_all(patRoot);
}

inline void test_collector(void) {
	collector_n::Collector_test().test();
}

inline void test(void) {
#ifdef TEST_GENERATOR_FILES
	test_gen(true);
#endif //TEST_GENERATOR_FILES
#ifdef TEST_COLLECTOR
	test_collector();
#endif //TEST_COLLECTOR
}


}
#endif /* TEST_H_ */
