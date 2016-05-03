/*
 * Storage.cpp
 *
 *  Created on: 23 апр. 2015 г.
 *      Author: andrej
 */

#include <iostream>
#include <sstream>
#include <boost/format.hpp>
#include "StorageMad.h"
#include <algorithm>
#include <cstring>
#include <thread>
#include <sstream>
#include <tuple>
#include <utility>
#include <stdexcept>

namespace genfiles_n {
using namespace boost::gregorian;

inline int counFilesInDirectory(path dir) {	//подсчитывает количество файлов в директории
	int count = 0;
	if(is_directory(dir)) {
		directory_iterator dirIterator(dir), end;
		count = std::count_if(dirIterator, end, [](const directory_entry& entry) {return is_regular_file(entry.path());});
	}
	return count;
}
std::string StorageMad::makeDirCurDate() {
	std::stringstream ss;
	date_facet* pFacet = new date_facet("%Y/%b/%d");
	ss.imbue(std::locale{ss.getloc(), pFacet});
	ss << curDay_;
	path patNewDir = root_ / ss.str();
	if(!is_directory(patNewDir)) {
		create_directories(patNewDir);
		if(!is_directory(patNewDir))
			return std::string();
	}
	return patNewDir.string();
}

int StorageMad::getNumberLastArchiveOfDir(const path& dir) const {
	int maxVal = 0;
	std::string strVal;
	if(!is_directory(dir)) {
		std::cerr << dir << " это не директория\n";
		exit(1);
	}
	directory_iterator dirIterator(dir), end;
	for (; dirIterator != end; ++ dirIterator) {
		if(is_regular_file(dirIterator->path()) && extension(dirIterator->path()) == SUFFIX_FOR_ARCHIVE) {
			strVal = dirIterator->path().stem().string();
			try {
				int val = std::stoi(strVal);
				if(val > maxVal)
					maxVal = val;
			}
			catch(...) {}
		}
	}
	return maxVal;
}

bool StorageMad::createNewDirCountAndCd(path& parent) {
	date day = day_clock::local_day();
	if(curDay_ != day) {
		curDay_ = day;
		std::string dirDay = makeDirCurDate();
		if(dirDay.empty())
			return false;
		parent = dirDay;
		curArchive_ = 0;
	}
	path newDirCount  = parent / std::to_string(++curArchive_);
	bool ok = true;
	if(!is_directory(newDirCount))
		ok = create_directory(newDirCount);
	if(ok)
		parent = newDirCount;
	else
		std::cerr << "Unable to create directory" << newDirCount << std::endl;
	numFile_ = 0;
	return ok;
}

bool StorageMad::isEqualityMetaData(std::shared_ptr<GenFiles> pFile1,
		std::shared_ptr<GenFiles> pFile2) const {
	if(pFile1 == nullptr || pFile2 == nullptr)
		return false;
	else
		return pFile1->isEqualMeta(*pFile2);
}

StorageMad::StorageMad(std::string rootDir, bool isSingleFileInArchDir): root_(rootDir), curDir_(root_),
		isSingleFileInArchDir_(isSingleFileInArchDir){
	if(!is_directory(root_)) {
		std::cerr << "Невозможно использовать корневую папку " << rootDir << std::endl;
		exit(1);
	}
	curDay_ = day_clock::local_day();
	std::string pathCurDir = makeDirCurDate();
	if(pathCurDir.empty()) {
		std::cerr << "Невозможно использовать текущую папку " << pathCurDir << std::endl;
		exit(1);
	}
	curDir_ = pathCurDir;
	curArchive_ = getNumberLastArchiveOfDir(curDir_);
	if(!createNewDirCountAndCd(curDir_))
		exit(1);
}

void StorageMad::makeArchiveAndCdNewDirectory( std::shared_ptr<GenFiles> pFileForMeta) {
	path dir = curDir_;
	if(counFilesInDirectory(curDir_) == 0) {
		curDir_ = curDir_.parent_path();
		curArchive_--;
		remove(dir);
	} else {
		if(!pFileForMeta->createFileMeta(curDir_))
			std::cerr << boost::format("В директории %1% невозможно сохранить файл %2%\n") % curDir_.string() % pFileForMeta->getMetaFileName();
		curDir_ = curDir_.parent_path();
		makeArchive(dir);
	}
	createNewDirCountAndCd(curDir_);
}

void archive(path dir, std::string strSuf) {
	boost::format strCommand("zip -rjq %1%%2% %1%");
	system((str(strCommand % dir.string() % strSuf)).c_str());
	remove_all(dir);
}

void StorageMad::makeArchive(const path& nameDirForArchive) {
	manager_.run(archive, nameDirForArchive, SUFFIX_FOR_ARCHIVE);
}

StorageMad::~StorageMad() {
	if(is_directory(curDir_)) {
		if(counFilesInDirectory(curDir_) == 0) {
			remove(curDir_);
		} else {
			if(!pPrevFile_->createFileMeta(curDir_))
				std::cerr << boost::format("В директории %1% невозможно сохранить файл %2%\n") % curDir_.string() % pPrevFile_->getMetaFileName();
			makeArchive(curDir_);
		}
	}
	manager_.waitForExec();
}

void StorageMad::save(std::shared_ptr<GenFiles> pFile) {
	if(pPrevFile_ != nullptr) {
		if(curDay_ != day_clock::local_day())
			makeArchiveAndCdNewDirectory(pPrevFile_);
		else if(!isEqualityMetaData(pPrevFile_, pFile)) {
			makeArchiveAndCdNewDirectory(pPrevFile_);
//#ifdef _DEBUG
//			std::cerr << boost::format("Директория %1% арх. из-за того, что метаданные файла %2% отличаются от файла %3%")
//			% curDir_.string() % pFile->getFileName() % pPrevFile_->getFileName() << std::endl;
//#endif //_DEBUG
		}
	}
	if(isSingleFileInArchDir_) {
		if(!pFile->createFileData(curDir_))
			std::cerr << boost::format("В директории %1% невозможно сохранить файл %2%\n") % curDir_.string() % pFile->getFileName();
	} else {
		if(!pFile->createFileData(curDir_, std::to_string(++numFile_) + "_"))
			std::cerr << boost::format("В директории %1% невозможно сохранить файл %2%\n") % curDir_.string()
			% (std::to_string(numFile_) + "_" +pFile->getFileName());
		if(counFilesInDirectory(curDir_) >= MAX_NUMBER_FILES_IN_DIR)
			makeArchiveAndCdNewDirectory(pFile);
	}
	pPrevFile_ = pFile;
}

Storage::Storage(const std::string& dir, bool isSingleFileInArchDir, int numMads): root_(dir) {
	if(!is_directory(root_))
		assert(create_directories(root_));
	for(int i = 1; i <= numMads; i++) {
		path patMad = root_ / (boost::format("mad%d") % i).str();
		if(!is_directory(patMad))
			assert(create_directories(patMad));
		poolMads_.emplace(std::piecewise_construct, std::make_tuple(i),std::make_tuple(patMad.string(), isSingleFileInArchDir));
	}
}

void Storage::save(std::shared_ptr<GenFiles> pFile) {
	try {
		poolMads_.at(pFile->getNumMad()).save(pFile);
	}   catch (const std::out_of_range& oor) {
		std::cerr << "Передан генератор файлов для МАДа, которого нет в хранилище: " << oor.what() << '\n';
	}
}

StorageManager::StorageManager(std::string dirName): root_(dirName) {
	if(!is_directory(root_))
		assert(create_directories(root_));
}

void StorageManager::addStorage(int idStorage, int numMads, const std::string& dir, bool isSingleFileInArchDir) {
	path patStorage = root_ / dir;
	if(!is_directory(patStorage))
		assert(create_directories(patStorage));
	auto pos = pool_.find(idStorage);
	if(pos != pool_.end())
		pool_.erase(pos);
	pool_.emplace(std::piecewise_construct, std::make_tuple(idStorage),std::make_tuple(patStorage.string(), isSingleFileInArchDir, numMads));

}

void StorageManager::save(int idStorage, std::shared_ptr<GenFiles> pFile) {
	try {
		pool_.at(idStorage).save(pFile);
	}   catch (const std::out_of_range& oor) {
		std::cerr << "Передан генератор файлов для хранилища, который не управляется менеджером хранилищ: " << oor.what() << '\n';
	}
}

StorageManager::~StorageManager() {
}

} /* namespace genfiles_n */

