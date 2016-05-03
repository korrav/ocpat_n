/*
 * GenGasikFile.h
 *
 *  Created on: 22 апр. 2015 г.
 *      Author: andrej
 */

#ifndef GENGASIKFILE_H_
#define GENGASIKFILE_H_

#include <GenFiles.h>
#include "GenFiles_test.h"

namespace genfiles_n {

class GenGasikFile: public GenFiles {
public:
	typedef short Sample;
	struct Params {
		unsigned int level;
	};
	GenGasikFile(const Head& head, const Params& params, const PtrData& buf);
	virtual ~GenGasikFile();
	virtual bool fillInFile(const std::string& file) const;	//запись данных в файл
	virtual bool fillInMetaFile(const std::string&  file) const;	//запись данных в метафайл
	virtual bool isEqualMeta(const GenFiles& gen) const;	//сравнение на равенство метаинформации
private:
	Params params_;
	PtrData buf_;
};

} /* namespace genfiles_n */

#endif /* GENGASIKFILE_H_ */
