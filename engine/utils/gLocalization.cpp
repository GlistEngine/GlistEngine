/*
 * gLocalization.cpp
 *
 *  Created on: 6 Tem 2021
 *      Author: Admin
 */

#include "gLocalization.h"
#include "gDatabase.h"



gLocalization::gLocalization() {
}

gLocalization::~gLocalization() {
	localizedb.close();
}


void gLocalization::loadDatabase(std::string database, std::string tableName) {
	//Database will open for reading
	tablename = tableName;
	currentlanguage = 0;
	starttowrite = false;
	localizedb.loadDatabase(database);
	selectquery = "SELECT sql FROM sqlite_master WHERE tbl_name = \'" + tablename + "\' AND type = \'table\'";
	localizedb.execute(selectquery);
	backupstring = localizedb.getSelectData();
	columnLister(backupstring);
}

std::string gLocalization::getTableInfo(){
	selectquery = "SELECT sql FROM sqlite_master WHERE tbl_name = \'" + tablename + "\' AND type = \'table\'";
	localizedb.execute(selectquery);
	backupstring = localizedb.getSelectData();
	return backupstring;
}

void gLocalization::columnLister(std::string tableinfo) {
	std::string temporarystring = "";

	for (int i = 0; i < backupstring.length(); i++){
		if (backupstring[i] == '\n'){
			temporarystring = "";
			i++;
			while(backupstring[i] != '\n' && i < backupstring.length()){
				if(backupstring[i] == '"') {
					starttowrite = !starttowrite;
					i++;
				}
				if(starttowrite)temporarystring += backupstring[i];
				i++;
			}
			i--;
			columnlist.push_back(temporarystring);
		}
	}

	for (std::string x : columnlist){
		for (int i = 0; i < x.length(); i++){
			if (x[i] == '\"'){
				i++;
				temporarystring = "";

				while (x[i] != '\"' && i < x.length()){
					temporarystring += x[i];
					i++;
				}
				if(temporarystring != "")columnlist.push_back(temporarystring);
			}
		}
	}
}

std::vector<std::string> gLocalization::getColumnList() {
	return columnlist;
}


void gLocalization::setCurrentLanguage(int languageId) {
	currentlanguage = languageId;
}


std::string gLocalization::getCurrentLanguage() {
	return columnlist[currentlanguage + 1];
}


int gLocalization::getLangId() {
	return currentlanguage;
}


std::string gLocalization::getColumnData(std::string columnname) {
	selectquery = "SELECT "+ columnname + " from " + tablename;
	localizedb.execute(selectquery);
	while(localizedb.getSelectData() != "") {
		resultstring = localizedb.getSelectData();
	}
	return resultstring;
}


std::string gLocalization::localizeWord(std:: string word) {
	selectquery = "SELECT " + columnlist[currentlanguage + 1] + " FROM " + tablename + " WHERE Key = \'" + word + "\'";
	localizedb.execute(selectquery);
	localizedword = gSplitString(localizedb.getSelectData(), "|")[1];
	return localizedword;
}

