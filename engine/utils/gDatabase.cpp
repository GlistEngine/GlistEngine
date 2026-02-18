/*
 * gDatabase.cpp
 *
 *  Created on: May 13, 2021
 *      Author: noyan
 */

#include "gDatabase.h"
#include <filesystem>

std::queue<std::string> gDatabase::selectdata;
int gDatabase::num;
std::string gDatabase::delimiter = "|";


gDatabase::gDatabase() {
	fullpath = "";
	db = nullptr;
}

gDatabase::~gDatabase() {
    if (db != nullptr) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool gDatabase::load(const std::string& fullPath) {

    if (db != nullptr) {
        sqlite3_close(db);
        db = nullptr;
    }

    fullpath = fullPath;

    sqlite3* newdb = nullptr;

#ifdef _WIN32
    std::wstring wpath = std::__fs::filesystem::path(fullpath).wstring();

    int rc = sqlite3_open16(
        wpath.c_str(),
        &newdb
    );
#else
    int rc = sqlite3_open_v2(
        fullpath.c_str(),
        &newdb,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
        nullptr
    );
#endif

    db = newdb;
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
	while (!selectdata.empty())
	    selectdata.pop();


	char* zErrMsg = 0;
	int rc = sqlite3_exec(db, statement.c_str(), gDatabase::callback, (void*)id.c_str(), &zErrMsg);
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
//	zsql = "SELECT * FROM sqlite_master WHERE type = 'table'";
//	execute(zsql);
//	gLogi("gDatabase") << "getTableInfo 2";
}


std::vector<std::string> gDatabase::getTableNames() {
	std::vector<std::string> data;
	std::string temporarystring = "";
	std::vector<std::string> datastorage;
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
    std::string res;

    if(statementId != nullptr)
        res = (char*)statementId;

    for(int i = 0; i < argc; i++) {

        res += delimiter;

        if(argv[i] != nullptr)
            res += argv[i];
        else
            res += "";
    }

    selectdata.push(res);
    return 0;
}
