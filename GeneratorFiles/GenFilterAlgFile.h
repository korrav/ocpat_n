/*
 * GenFilterAlgFile.h
 *
 *  Created on: 18 апр. 2015 г.
 *      Author: andrej
 */

#ifndef GENFILTERALGFILE_H_
#define GENFILTERALGFILE_H_

#include "GeneratorFiles.h"
#include "GenFiles_test.h"

namespace genfiles_n {

class GenFilterAlgFile: public GenFiles {
public:
	typedef short Sample;
	struct Params {
		float sigma;
	};
	GenFilterAlgFile(const Head& head, const Params& params, const PtrData& buf);
	virtual ~GenFilterAlgFile();
	virtual bool fillInFile(const std::string& file) const;	//запись данных в файл
	virtual bool fillInMetaFile(const std::string&  file) const;	//запись данных в метафайл
	virtual bool isEqualMeta(const GenFiles& gen) const;	//сравнение на равенство метаинформации
private:
	Params params_;
	PtrData buf_;
};

} /* namespace genfiles_n */

#endif /* GENFILTERALGFILE_H_ */
