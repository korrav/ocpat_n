/*
 * GenGasikFile_test.cpp
 *
 *  Created on: 22 апр. 2015 г.
 *      Author: andrej
 */

#include "GenGasikFile_test.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <assert.h>
#include "Test.h"

namespace genfiles_n {
void TestGasikGenFiles::testCompareMeta(void) {
	GenFiles::Head head1 = TestGenFiles(1).getHead(1);
	GenFiles::Head head2 = TestGenFiles(2).getHead(2);
	GenGasikFile::Params params1, params2;
	params1.level = 5;
	params2.level = 6;
	GenGasikFile gen1(head1, params1, getBuffer()), gen2(head1, params1, getBuffer());
	gen1.setFileName(test_n::getCurrentTimeStr());
	gen2.setFileName(test_n::getCurrentTimeStr());
	assert(gen1.isEqualMeta(gen2));
	GenGasikFile gen3(head1, params1, getBuffer()), gen4(head1, params2, getBuffer());
	gen3.setFileName(test_n::getCurrentTimeStr());
	gen4.setFileName(test_n::getCurrentTimeStr());
	assert(!gen3.isEqualMeta(gen4));
	GenGasikFile gen5(head1, params1, getBuffer()), gen6(head2, params1, getBuffer());
	gen5.setFileName(test_n::getCurrentTimeStr());
	gen6.setFileName(test_n::getCurrentTimeStr());
	assert(!gen5.isEqualMeta(gen6));
}

PtrData TestGasikGenFiles::getBuffer() {
	using namespace boost::posix_time;
	int numberSample = 1000;
	size_t  size = numberSample * sizeof(GenGasikFile::Sample);
	PtrData ptrBuf(new PtrData::element_type(size));
	GenGasikFile::Sample* pSampl = reinterpret_cast<GenGasikFile::Sample*>(ptrBuf->data());
	for(int i = 0; i < numberSample; i++)
		pSampl[i] = i;
	return ptrBuf;
}

void TestGasikGenFiles::testCreateFiles(void) {
	using namespace boost::filesystem;
	GenFiles::Head head = TestGenFiles(1).getHead(1);
	GenGasikFile::Params params;
	params.level = 5;
	GenGasikFile gen(head, params, getBuffer());
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

void TestGasikGenFiles::test(void) {
	testCompareMeta();
	testCreateFiles();
}

TestGasikGenFiles::TestGasikGenFiles(const std::string& dir): TEST_DIRECTORY_(dir) {
}
} /* namespace genfiles_n */
