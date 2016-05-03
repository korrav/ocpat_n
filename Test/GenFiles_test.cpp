/*
 * GenFiles_test.cpp
 *
 *  Created on: 11 апр. 2015 г.
 *      Author: andrej
 */

#include "GenFiles_test.h"
#include <assert.h>
#include <boost/filesystem.hpp>

namespace genfiles_n {

using namespace boost::filesystem;

TestGenFiles::TestGenFiles(unsigned numMads, const std::string& dir): TEST_DIRECTORY_(dir) {
	assert(numMads > 0);
	for(unsigned i = 1; i <= numMads; i++)
		headMads_[i] = buildHead(i);
}

TestGenFiles::Head TestGenFiles::getHead(int mad) {
	assert(static_cast<unsigned>(mad) <= headMads_.size());
	return headMads_[mad];

}

TestGenFiles::Head TestGenFiles::getDifferentHead(int mad) {
	Head head = getHead(mad);
	head.freq += 1;
	return head;
}

void TestGenFiles::test(void) {
	testCompareHead();
	testCreateMetaFile(TEST_DIRECTORY_);
}

TestGenFiles::Head TestGenFiles::buildHead(int mad) {
	Head head;
	head.verSoft = 1;
	head.numMad = mad;
	head.verHard = 1;
	head.freq = 600000;
	for(int i = 0; i < 4; i++)
		head.gain[i] = 34;
	for(int hyd = 0; hyd < 4; hyd++)
		for(int param = 0; param < NUM_PARAM_HYD; param++)
			head.coordHyd[hyd][param]= param;
	for(int hyd = 0; hyd < 4; hyd++)
		for(int param = 0; param < NUM_PARAM_AFC; param++)
			head.afc[hyd][param]= param;
	for(int i = 0; i < 4; i++)
		head.numHyd[i] = (mad -1) * 4 + i + 1;
	head.numAlg = 1;
	head.verAlg = 1;
	return head;
}

void TestGenFiles::testCompareHead(void) {
	Head head1 = getHead(headMads_.size());
	GenFiles gen1(head1);
	Head head2 = getDifferentHead(headMads_.size());
	GenFiles gen2(head2);
	assert(!gen1.isEqualMeta(gen2));
}

TestGenFiles::~TestGenFiles() {
}

void TestGenFiles::testCreateMetaFile(const std::string& dir) {
	GenFiles gen(getHead(headMads_.size()));
	path p(dir);
	assert(is_directory(p));
	p /= gen.getMetaFileName();
	assert(gen.fillInMetaFile(p.string()));
}

} /* namespace genfiles_n */

