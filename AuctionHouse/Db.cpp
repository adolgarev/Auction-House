#include "Db.h"

namespace auction {

	DbTransaction Db::startTransaction()
	{
		return DbTransaction(this);
	}

	void Db::commitTransaction()
	{
	}

	void Db::rollbackTransaction()
	{
	}

}