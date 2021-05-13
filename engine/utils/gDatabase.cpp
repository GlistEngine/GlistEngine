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

bool gDatabase::open(std::string fullPath) {
	fullpath = fullPath;
	int rc = sqlite3_open(fullpath.c_str(), &db);
	if(rc) {
	   gLoge("gDatabase") << "Can't open database! errormsg:" << sqlite3_errmsg(db);
	  return false;
	}
	return true;
}

bool gDatabase::openDatabase(std::string databasePath) {
	return open(gGetDatabasesDir() + databasePath);
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
	  fprintf(stderr, "SQL error: %s\n", zErrMsg);
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

int gDatabase::callback(void* statementId, int argc, char **argv, char **azColName) {
	std::string res = gToStr((char*)statementId);
	for(int i = 0; i < argc; i++) res += delimiter + std::string(argv[i]);
	selectdata.push(res);
	return 0;
}


