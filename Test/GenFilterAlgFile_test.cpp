/*
 * GenFilterAlgFile_test.cpp
 *
 *  Created on: 18 апр. 2015 г.
 *      Author: andrej
 */

#include "GenFilterAlgFile_test.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <assert.h>
#include "Test.h"

namespace genfiles_n {

void TestFilterAlgGenFiles::testCompareMeta(void) {
	GenFiles::Head head1 = TestGenFiles(1).getHead(1);
	GenFiles::Head head2 = TestGenFiles(2).getHead(2);
	GenFilterAlgFile::Params params1, params2;
	params1.sigma = 5.5;
	params2.sigma = 6.6;
	GenFilterAlgFile gen1(head1, params1, getBuffer()), gen2(head1, params1, getBuffer());
	gen1.setFileName(test_n::getCurrentTimeStr());
	gen2.setFileName(test_n::getCurrentTimeStr());
	assert(gen1.isEqualMeta(gen2));
	GenFilterAlgFile gen3(head1, params1, getBuffer()), gen4(head1, params2, getBuffer());
	gen3.setFileName(test_n::getCurrentTimeStr());
	gen4.setFileName(test_n::getCurrentTimeStr());
	assert(!gen3.isEqualMeta(gen4));
	GenFilterAlgFile gen5(head1, params1, getBuffer()), gen6(head2, params1, getBuffer());
	gen5.setFileName(test_n::getCurrentTimeStr());
	gen6.setFileName(test_n::getCurrentTimeStr());
	assert(!gen5.isEqualMeta(gen6));
}

PtrData TestFilterAlgGenFiles::getBuffer() {
	using namespace boost::posix_time;
	int numberSample = 1000;
	size_t  size = numberSample * sizeof(GenFilterAlgFile::Sample);
	PtrData ptrBuf(new PtrData::element_type(size));
	GenFilterAlgFile::Sample* pSampl = reinterpret_cast<GenFilterAlgFile::Sample*>(ptrBuf->data());
	for(int i = 0; i < numberSample; i++)
		pSampl[i] = i;
	return ptrBuf;
}

void TestFilterAlgGenFiles::testCreateFiles(void) {
	using namespace boost::filesystem;
	GenFilterAlgFile gen = getGen(1);
	gen.setFileName(test_n::getCurrentTimeStr());
	path p(TEST_DIRECTORY_);
	assert(is_directory(p));
	p /= gen.getMetaFileName();
	assert(gen.fillInMetaFile(p.string()));
	p = TEST_DIRECTORY_;
	p /= gen.getFileName();
	assert(gen.fillHeadlineInFile(p.string()));
	assert(gen.fillInFile(p.string()));
}

void TestFilterAlgGenFiles::test(void) {
	testCompareMeta();
	testCreateFiles();
}

GenFilterAlgFile TestFilterAlgGenFiles::getGen(int numMad) {
	using namespace boost::filesystem;
	GenFiles::Head head = TestGenFiles(numMad).getHead(numMad);
	GenFilterAlgFile::Params params;
	params.sigma = 5.5;
	GenFilterAlgFile gen(head, params, getBuffer());
	gen.setFileName(test_n::getCurrentTimeStr());
	return gen;
}

GenFilterAlgFile TestFilterAlgGenFiles::getOtherGen(int numMad) {
	using namespace boost::filesystem;
	GenFiles::Head head = TestGenFiles(numMad).getHead(numMad);
	GenFilterAlgFile::Params params;
	params.sigma = 6.5;
	GenFilterAlgFile gen(head, params, getBuffer());
	gen.setFileName(test_n::getCurrentTimeStr());
	return gen;
}

TestFilterAlgGenFiles::TestFilterAlgGenFiles(const std::string& dir): TEST_DIRECTORY_(dir) {
}

} /* namespace genfiles_n */
