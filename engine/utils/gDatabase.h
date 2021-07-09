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

	bool load(std::string fullPath);
	bool loadDatabase(std::string databasePath);
	void close();

	std::string getPath();

	void execute(std::string statement, std::string id = "0");
	static int callback(void *statementId, int argc, char **argv, char **azColName);

	std::string getSelectData();
	bool hasSelectData();
	int getSelectDataNum();

	void setDelimiter(std::string delimiter);
	std::string getDelimiter();

private:
	sqlite3* db;
	std::string fullpath;
	static int num;
	static std::queue<std::string> selectdata;
	static std::string delimiter;
};

#endif /* UTILS_GDATABASE_H_ */
