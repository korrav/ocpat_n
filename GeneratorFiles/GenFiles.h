/*
 * GenFiles.h
 *
 *  Created on: 11 апр. 2015 г.
 *      Author: andrej
 */

#ifndef GENFILES_H_
#define GENFILES_H_

#include <string>
//#include "Detector.h"
#include <boost/filesystem.hpp>

namespace genfiles_n {

#define NUM_PARAM_HYD 3
#define NUM_PARAM_AFC 3

using namespace boost::filesystem;

class GenFiles {
	const char* NAME_METAFILE = "Meta";
	const char* NAME_DATEFILE = "Data";
	std::string nameFile_ = NAME_DATEFILE;
public:
	struct Head {
		int32_t verSoft;	//версия формата пакета и файла метаданных
		int32_t numMad;	//номер МАДа
		int32_t verHard;	//версия аппаратуры
		int32_t freq;	//частота дискретизации
		int32_t gain[4];	//коэффициенты усиления в каналах
		int32_t coordHyd[4][NUM_PARAM_HYD];	//координаты гидрофонов МАДа
		int32_t afc[4][NUM_PARAM_AFC];	//АЧХ
		int32_t numHyd[4];	//номера гидрофонов
		int32_t numAlg;	//номер Алгоритма
		int32_t verAlg;	//версия алгоритма
	};

	GenFiles(const Head&);
	virtual ~GenFiles();
	virtual std::string getFileName() const;	//возвратить имя файла данных
	virtual void setFileName(std::string name);	//установить имя файла данных
	virtual bool fillHeadlineInFile(const std::string& file) const;	//запись заголовка файла данных
	virtual bool fillInFile(const std::string& file) const;	//запись данных в файл
	virtual std::string getMetaFileName() const;	//возвратить имя мета файла данных
	virtual bool fillInMetaFile(const std::string&  file) const;	//запись данных в метафайл
	virtual bool isEqualMeta(const GenFiles&) const;	//сравнение на равенство метаинформации
	int32_t getNumMad(void) const;	//возвратить номер мада
	bool createFileData(const path& dir, const std::string& prefix = "", const std::string& suffix = "");	//создать файл данных в директории
	bool createFileMeta(const path& dir, const std::string& prefix = "", const std::string& suffix = "");	//создать файл метаданных в директории
protected:
	Head head_;
};

} /* namespace genfiles_n */

#endif /* GENFILES_H_ */
