/*
 * Copyright (C) 2016 Nitra Games Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UTILS_GDATABASE_H_
#define UTILS_GDATABASE_H_

#include <queue>
#include "sqlite3.h"
#include "gObject.h"


class gDatabase: public gObject {
public:
	gDatabase();
	virtual ~gDatabase();

	bool load(const std::string& fullPath);
	bool loadDatabase(std::string databasePath);
	void close();

	std::string getPath();

	void execute(std::string statement, std::string id = "0");
	static int callback(void *statementId, int argc, char **argv, char **azColName);

	std::string getSelectData();
	bool hasSelectData();
	int getSelectDataNum();

	/*
	 * Recieves all table infos from a database
	 *
	 * By sending variable pointers this method will give you all table infos from a database.
	 *
	 * @return since you sending pointer as input, it will modify and alter variables with results so you can directly use same variable.
	 */
	void getTableInfo(char*** sqlResult, int* rowNum, int* colNum);

	void setDelimiter(std::string delimiter);
	std::string getDelimiter();

	/*
	 * Receives table names from database
	 *
	 * This metod give you table lists in a database.
	 *
	 * Using sql query for execute() metod and getting table list as result
	 *
	 * @param data which table to get info so metod will get full column list
	 * @param datastorage selecting column to recieve which type of this column name
	 * @param temporarystring storing table names which metod finds
	 * @param data returning tablenames info as vector
	 *
	 * @return resultstring return back column type of columnName
	 */
	std::vector<std::string> getTableNames();

	/* Gives all column names from table info
	 *
	 * Receives column names from @tableName after database loaded.
	 *
	 * After load database and give table name as input metod store all column names of that table
	 *
	 * @return sending back colunm name list as string vector
	 */
	std::vector<std::string> getColumnNames(std::string tableName);

	/*
	 * Receives columnType from given colunmName and tableName
	 *
	 * This metod give you column type from senden table name and column name.
	 *
	 * @param tableName which table to get info so metod will get full column list
	 * @param columnName selecting column to recieve which type of this column name
	 *
	 * @return resultstring return back column type of columnName
	 */
	std::string getColumnType(std::string tableName, std::string columnName);
private:
	sqlite3* db;
	std::string fullpath;
	static int num;
	static std::queue<std::string> selectdata;
	static std::string delimiter;
	//parameters of
	std::string zsql;
	int rows;
	int cols;
	char **result;
	int kt;
	int pipecounter = 0;
	std::string** resultstring;
	std::vector<std::string> columnList;
};

#endif /* UTILS_GDATABASE_H_ */
