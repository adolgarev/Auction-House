#include "easylogging++.h"
#include "Db.h"

namespace auction {

	DbTransaction Db::startTransaction()
	{
		LOG(DEBUG) << "StartTransaction";
		return DbTransaction(this);
	}

	void Db::commitTransaction()
	{
		LOG(DEBUG) << "CommitTransaction";
	}

	void Db::rollbackTransaction()
	{
		LOG(DEBUG) << "RollbackTransaction";
	}

}