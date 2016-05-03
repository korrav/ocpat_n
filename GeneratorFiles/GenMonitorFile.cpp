/*
 * GenMonitorFile.cpp
 *
 *  Created on: 15 апр. 2015 г.
 *      Author: andrej
 */

#include "GenMonitorFile.h"
#include <iomanip>
#include <fstream>
#include <sstream>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace genfiles_n {

GenMonitorFile::GenMonitorFile(const Head& head, const Monitor& monit): GenFiles(head), monit_(monit) {
}

GenMonitorFile::~GenMonitorFile() {
}

std::string GenMonitorFile::getFileName() const {
	return NAME_MONITORFILE;
}

bool GenMonitorFile::fillHeadlineInFile(const std::string& file) const {
	std::ofstream out(file, std::ofstream::out | std::ofstream::app);
	if(!out.is_open())
		return false;
	out.setf(std::ios::left);
	out << std::setw(WIDTH_FIELD) << "time";
	for(int i = 0; i < 4; i++)
		out << std::setw(WIDTH_FIELD) << std::string("mean_ch") + std::to_string(i);
	for(int i = 0; i < 4; i++)
		out << std::setw(WIDTH_FIELD) << std::string("rms_ch") + std::to_string(i);
	out << std::endl;
	out.close();
	return true;
}

bool GenMonitorFile::fillInFile(const std::string& file) const {
	using namespace boost::posix_time;
	std::ofstream out(file, std::ofstream::out | std::ofstream::app);
	if(!out.is_open())
		return false;
	if(file_size(path(file)) == 0)
		fillHeadlineInFile(file);
	std::stringstream ss;
	ptime time = second_clock::local_time();
	time_facet* pFacet = new time_facet("%d%m%y_%H%M%S");
	ss.imbue(std::locale{out.getloc(), pFacet});
	ss << time;
	out.setf(std::ios::left);
	out << std::setw(WIDTH_FIELD) << ss.str();
	for(int i = 0; i < 4; i++)
		out << std::setw(WIDTH_FIELD) << monit_.mean[i];
	for(int i = 0; i < 4; i++)
		out << std::setw(WIDTH_FIELD) << monit_.rms[i];
	out << std::endl;
	out.close();
	return true;
}

} /* namespace genfiles_n */
