/*
 * gDatabase.cpp
 *
 *  Created on: May 13, 2021
 *      Author: noyan
 */

#include "gDatabase.h"

std::queue<std::string> gDatabase::selectdata;
int gDatabase::num;
std::string gDatabase::delimiter = "|";


gDatabase::gDatabase() {
	fullpath = "";
}

gDatabase::~gDatabase() {
}

bool gDatabase::load(std::string fullPath) {
	fullpath = fullPath;
	int rc = sqlite3_open(fullpath.c_str(), &db);
	if(rc) {
	   gLoge("gDatabase") << "Can't open database! errormsg:" << sqlite3_errmsg(db);
	  return false;
	}
	return true;
}

bool gDatabase::loadDatabase(std::string databasePath) {
	return load(gGetDatabasesDir() + databasePath);
}

void gDatabase::close() {
	sqlite3_close(db);
}

std::string gDatabase::getPath() {
	return fullpath;
}

void gDatabase::setDelimiter(std::string delimiter) {
	gDatabase::delimiter = delimiter;
}

std::string gDatabase::getDelimiter() {
	return delimiter;
}


void gDatabase::execute(std::string statement, std::string id) {
	char* zErrMsg = 0;
	int rc = sqlite3_exec(db, statement.c_str(), gDatabase::callback, (void*)id.c_str(), &zErrMsg);

	if(rc != SQLITE_OK){
		gLoge("gDatabase") << "SQL error:" << zErrMsg << ", statement: " << statement;
		sqlite3_free(zErrMsg);
	}
}

int gDatabase::getSelectDataNum() {
	return selectdata.size();
}

bool gDatabase::hasSelectData() {
	return selectdata.size();
}

std::string gDatabase::getSelectData() {
	if (selectdata.empty()) return "";

	std::string res = selectdata.front();
	selectdata.pop();
	return res;
}

void gDatabase::getTableInfo(char*** sqlResult, int* rowNum, int* colNum) {
	zsql = "SELECT * FROM sqlite_master WHERE type = 'table'";
	execute(zsql);
	gLogi("gDatabase") << "getTableInfo 2";
}


std::vector<std::string> gDatabase::getTableNames() {
	std::vector<std::string> data;
	std::string temporarystring = "";
	std::vector<std::string> datastorage;
	zsql = "SELECT * FROM sqlite_master WHERE type = 'table'";
	execute(zsql);


	while(!selectdata.empty()) {
		temporarystring = "";
		temporarystring = selectdata.front();
		selectdata.pop();
		//char read
		datastorage = gSplitString(temporarystring, "|");
		data.push_back(datastorage[2]);
	}
	return data;
}


std::string gDatabase::getColumnType(std::string tableName, std::string columnName) {
	zsql = "SELECT sql FROM sqlite_master WHERE tbl_name = \'" + tableName + "\' AND type = \'table\'";
	execute(zsql);
	std::string backupstring = selectdata.front();
	selectdata.pop();
	std::string resultstring;
	int columnsearchpoint = 0;
	int startpoint = 0;
	int endpoint = 0;
	int searchpoint = 0;
		for(int i = 0; i < backupstring.length(); i++) {
			if(backupstring[i] == '"' && searchpoint == 0) {
				startpoint = i+1;
				searchpoint = 1;
				continue;
			}
			if(backupstring[i] == '"' && searchpoint == 1) {
				endpoint = i;
				searchpoint = 2;
			}
			if(searchpoint == 2) {
				if(backupstring.substr(startpoint, endpoint - startpoint) != tableName)resultstring = backupstring.substr(startpoint, endpoint - startpoint);
				//taking columntype
				if(resultstring == columnName) {
					for(int j = endpoint; j < backupstring.length(); j++) {
						if(backupstring[j] == '\t' && columnsearchpoint == 0) {
							startpoint = j+1;
							columnsearchpoint = 1;
							continue;
						}
						if(backupstring[j] == ',' && columnsearchpoint == 1) {
							endpoint = j;
							columnsearchpoint = 2;
						}
						if(columnsearchpoint == 2) {
							resultstring = backupstring.substr(startpoint, endpoint - startpoint);

							return resultstring;
						}
					}
				}
				searchpoint = 0;
				startpoint = 0;
				endpoint = 0;
			}
		}
		return "NULL";
}


std::vector<std::string> gDatabase::getColumnNames(std::string tableName) {
	zsql = "SELECT sql FROM sqlite_master WHERE tbl_name = \'" + tableName + "\' AND type = \'table\'";
	execute(zsql);
	std::string backupstring = selectdata.front();
	selectdata.pop();
	gLogi("columnlist: ") << backupstring;
	int startpoint = 0;
	int endpoint = 0;
	int searchpoint = 0;
	for(int i = 0; i < backupstring.length(); i++) {
		if(backupstring[i] == '"' && searchpoint == 0) {
			startpoint = i+1;
			searchpoint = 1;
			continue;
		}
		if(backupstring[i] == '"' && searchpoint == 1) {
			endpoint = i;
			searchpoint = 2;
		}
		if(searchpoint == 2) {
			if(backupstring.substr(startpoint, endpoint - startpoint) != tableName)columnList.push_back(backupstring.substr(startpoint, endpoint - startpoint));
			searchpoint = 0;
			startpoint = 0;
			endpoint = 0;
		}
	}

	return columnList;
}


int gDatabase::callback(void* statementId, int argc, char **argv, char **azColName) {
	std::string res = gToStr((char*)statementId);
	for(int i = 0; i < argc; i++) res += delimiter + std::string(argv[i]);
	selectdata.push(res);
	return 0;
}


