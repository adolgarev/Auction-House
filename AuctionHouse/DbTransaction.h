#pragma once

namespace auction {

	class Db;

	class DbTransaction
	{
		bool committed;
		Db* db;
	public:
		DbTransaction(Db* db) : db(db), committed(false) {}
		void commit();
		void rollback();
		~DbTransaction();
	};
}