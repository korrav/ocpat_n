/*
 * GenFiles.cpp
 *
 *  Created on: 11 апр. 2015 г.
 *      Author: andrej
 */

#include "GenFiles.h"
#include "boost/format.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
namespace genfiles_n {

GenFiles::GenFiles(const Head& head) {
	head_ = head;
}

GenFiles::~GenFiles() {

}

bool GenFiles::fillInMetaFile(const std::string& file) const {
	std::ofstream out(file, std::ofstream::out | std::ofstream::app);
	if(!out.is_open())
		return false;
	out << boost::format{"FORMAT_VER    %|20t|%1%\n"} % head_.verSoft;
	out << boost::format{"MAD   %|20t|%1%\n"} % head_.numMad;
	out << boost::format{"HARD_VER      %|20t|%1%\n"} % head_.verHard;
	out << boost::format{"FREQ     %|20t|%1%\n"} % head_.freq;
	for(int hyd =0; hyd < 4; hyd++){
		out << boost::format{"GAIN%1%   %|20t|%2%\n"} % (hyd+1) % head_.gain[hyd];
	}
	for(int hyd =0; hyd < 4; hyd++) {
		out << boost::format{"COORD_HYD%1%  %|20t|"} % (hyd + 1);
		for(int param = 0; param < NUM_PARAM_HYD; param++)
			out << std::left << std::setw(10) << head_.coordHyd[hyd][param];
		out << "\n";
	}
	for(int hyd =0; hyd < 4; hyd++) {
		out << boost::format{"AFC%1%   %|20t|"} % (hyd + 1);
		for(int param = 0; param < NUM_PARAM_AFC; param++)
			out << std::left << std::setw(10) << head_.afc[hyd][param];
		out << "\n";
	}
	out << boost::format{"NUMB_HYD   %|20t|%1$-10d%2$-10d%3$-10d%4$-10d\n"}
	% head_.numHyd[0] % head_.numHyd[1] % head_.numHyd[2] % head_.numHyd[3];
	out << boost::format{"ALG    %|20t|%1%\n"} % head_.numAlg;
	out << boost::format{"ALG_VER  %|20t|%1%\n"} % head_.verAlg;
	out.close();
	return true;
}

std::string GenFiles::getMetaFileName() const{
	return std::string(NAME_METAFILE);
}

std::string GenFiles::getFileName() const {
	return nameFile_;
}

void GenFiles::setFileName(std::string name) {
	nameFile_ = name;
}

bool GenFiles::fillHeadlineInFile(const std::string& file) const {
	return true;
}

bool GenFiles::isEqualMeta(const GenFiles& gen) const {
	if(head_.verSoft != gen.head_.verSoft)
		return false;
	if(head_.numMad != gen.head_.numMad)
		return false;
	if(head_.verHard != gen.head_.verHard)
		return false;
	if(head_.freq != gen.head_.freq)
		return false;
	for(int hyd = 0; hyd < 4; hyd++)
		if(head_.gain[hyd] != gen.head_.gain[hyd])
			return false;
	for(int hyd = 0; hyd < 4; hyd++)
		for(int param = 0; param < NUM_PARAM_HYD; param++)
			if(head_.coordHyd[hyd][param] != gen.head_.coordHyd[hyd][param])
				return false;
	for(int hyd = 0; hyd < 4; hyd++)
		for(int param = 0; param < NUM_PARAM_AFC; param++)
			if(head_.afc[hyd][param] != gen.head_.afc[hyd][param])
				return false;
	for(int hyd = 0; hyd < 4; hyd++)
		if(head_.numHyd[hyd] != gen.head_.numHyd[hyd])
			return false;
	if(head_.numAlg != gen.head_.numAlg)
		return false;
	if(head_.verAlg != gen.head_.verAlg)
		return false;
	return true;
}

bool GenFiles::fillInFile(const std::string& file) const {
	return false;
}
int32_t GenFiles::getNumMad(void) const{
	return head_.numMad;
}

bool GenFiles::createFileData(const path& dir, const std::string& prefix, const std::string& suffix) {
	std::string nameFile(prefix +getFileName() + suffix);
	if(nameFile.empty()) {
		std::cout << "Создаваемый файл имеет пустое имя" << std::endl;
		return false;
	}
	path file = dir / nameFile;
	return fillInFile(file.string());
}

bool GenFiles::createFileMeta(const path& dir, const std::string& prefix, const std::string& suffix) {
	std::string nameFile(prefix + getMetaFileName() + suffix);
	if(nameFile.empty()) {
		return false;
	}
	path file = dir / nameFile;
	return fillInMetaFile(file.string());
}

} /* namespace genfiles_n */
