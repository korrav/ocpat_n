/*
 * GenFilterAlgFile.cpp
 *
 *  Created on: 18 апр. 2015 г.
 *      Author: andrej
 */

#include <GenFilterAlgFile.h>
#include <fstream>
#include "boost/format.hpp"
#include <iostream>

namespace genfiles_n {

GenFilterAlgFile::GenFilterAlgFile(const Head& head, const Params& params, const PtrData& buf):
														GenFiles(head), params_(params), buf_(buf) {
}

GenFilterAlgFile::~GenFilterAlgFile() {
}

bool GenFilterAlgFile::fillInFile(const std::string& file) const {
	std::ofstream out(file, std::ofstream::out | std::ofstream::app | std::ofstream::binary);
	if(!out.is_open()) {
		std::cout << "Невозможно открыть файл " << file << " на запись" << std::endl;
		return false;
	}
	out.write(reinterpret_cast<char*>(buf_->data()), buf_->size());
	if(out.bad())
		return false;
	else
		return true;
}

bool GenFilterAlgFile::fillInMetaFile(const std::string& file) const {
	if(!GenFiles::fillInMetaFile(file))
		return false;
	std::ofstream out(file, std::ofstream::out | std::ofstream::app);
	if(!out.is_open())
		return false;
	out << boost::format{"SIGMA    %|20t|%1%\n"} % params_.sigma;
	out.close();
	return true;
}

bool GenFilterAlgFile::isEqualMeta(const GenFiles& gen) const {
	if(!GenFiles::isEqualMeta(gen))
		return false;
	const GenFilterAlgFile* genFilterAlg = dynamic_cast<const GenFilterAlgFile*>(&gen);
	if(genFilterAlg == nullptr)
		return false;
	return (params_.sigma == genFilterAlg->params_.sigma);
}

} /* namespace genfiles_n */
