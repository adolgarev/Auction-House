#include "DbTransaction.h"
#include "Db.h"

namespace auction {

	void DbTransaction::commit()
	{
		committed = true;
	}

	void DbTransaction::rollback()
	{
		committed = false;
	}

	DbTransaction::~DbTransaction()
	{
		if (committed)
			db->commitTransaction();
		else
			db->rollbackTransaction();
	}

}