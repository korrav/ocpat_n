/*
 * GenMonitorFile.h
 *
 *  Created on: 15 апр. 2015 г.
 *      Author: andrej
 */

#ifndef GENMONITORFILE_H_
#define GENMONITORFILE_H_

#include "GenFiles.h"

namespace genfiles_n {

class GenMonitorFile: public GenFiles {
	const char* NAME_MONITORFILE = "Monitor";
	const int WIDTH_FIELD = 16;
public:
	struct Monitor { //структура данных, в которую оборачиваются результаты вычисления статистики
		int rms[4]; 	//СКО
		int mean[4];	//математическое ожидание
	};

	GenMonitorFile(const Head& head, const Monitor& monit);
	virtual ~GenMonitorFile();
	virtual std::string getFileName() const;	//возвратить имя файла данных
	virtual bool fillHeadlineInFile(const std::string& file) const;	//запись заголовка файл данных
	virtual bool fillInFile(const std::string& file) const;	//запись данных в файл
private:
	Monitor monit_;
};

} /* namespace genfiles_n */

#endif /* GENMONITORFILE_H_ */
