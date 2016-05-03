/*
 * GenGasikFile.cpp
 *
 *  Created on: 22 апр. 2015 г.
 *      Author: andrej
 */

#include <GenGasikFile.h>
#include "boost/format.hpp"
#include <fstream>

namespace genfiles_n {

GenGasikFile::GenGasikFile(const Head& head, const Params& params, const PtrData& buf):
				GenFiles(head), params_(params), buf_(buf) {
}

GenGasikFile::~GenGasikFile() {
}

bool GenGasikFile::fillInFile(const std::string& file) const {
	std::ofstream out(file, std::ofstream::out | std::ofstream::app | std::ofstream::binary);
	if(!out.is_open())
		return false;
	out.write(reinterpret_cast<char*>(buf_->data()), buf_->size());
	if(out.bad())
		return false;
	else
		return true;
}

bool GenGasikFile::fillInMetaFile(const std::string& file) const {
	if(!GenFiles::fillInMetaFile(file))
		return false;
	std::ofstream out(file, std::ofstream::out | std::ofstream::app);
	if(!out.is_open())
		return false;
	out << boost::format{"LEVEL    %|20t|%1%\n"} % params_.level;
	out.close();
	return true;
}

bool GenGasikFile::isEqualMeta(const GenFiles& gen) const {
	if(!GenFiles::isEqualMeta(gen))
		return false;
	const GenGasikFile* genFilterAlg = dynamic_cast<const GenGasikFile*>(&gen);
	if(genFilterAlg == nullptr)
		return false;
	return (params_.level == genFilterAlg->params_.level);
}

} /* namespace genfiles_n */
