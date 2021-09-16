/*
 * gLocalization.cpp
 *
 *  Created on: 6 Tem 2021
 *      Author: Admin
 */

#include <gLocalization.h>
#include <gDatabase.h>



gLocalization::gLocalization() {
	// TODO Auto-generated constructor stub


}

gLocalization::~gLocalization() {
	// TODO Auto-generated destructor stub
	localizedb.close();
}


void gLocalization::loadDatabase(std::string database) {
	//Database will open for reading
	currentlanguage = 1;
	starttowrite = false;
	localizedb.loadDatabase("gamelocal.db");
	selectquery = "SELECT sql FROM sqlite_master WHERE tbl_name = 'WORDS' AND type = 'table'";
	localizedb.execute(selectquery);
	backupString = localizedb.getSelectData();
	columnLister(backupString);

}


std::string gLocalization::getTableInfo(){
	selectquery = "SELECT sql FROM sqlite_master WHERE tbl_name = 'WORDS' AND type = 'table'";
	localizedb.execute(selectquery);
	backupString = localizedb.getSelectData();
	return backupString;
}

void gLocalization::columnLister(std::string tableinfo) {
	std::string temporarystring = "";


	for (int i = 0; i < backupString.length(); i++){

		if (backupString[i] == '\n'){
			temporarystring = "";
			i++;
			while(backupString[i] != '\n' && i < backupString.length()){
				if(backupString[i] == '"') {
					starttowrite = !starttowrite;
					i++;
				}
				if(starttowrite)temporarystring += backupString[i];
				i++;
			}
			i--;
			columnList.push_back(temporarystring);

		}
	}


	for (std::string x : columnList){

		for (int i = 0; i < x.length(); i++){

			if (x[i] == '\"'){
				i++;
				temporarystring = "";

				while (x[i] != '\"' && i < x.length()){
					temporarystring += x[i];
					i++;
				}

				columnList.push_back(temporarystring);

			}
		}
	}
}


void gLocalization::setCurrentLanguage(int languageId) {
	currentlanguage = languageId;
}


std::string gLocalization::getCurrentLanguage() {
	return columnList[currentlanguage];
}


int gLocalization::getLangId() {
	return currentlanguage;
}


std::string gLocalization::getColumnData(std::string columnname) {//sorunu var
	selectquery = "SELECT "+ columnname + " from WORDS";
	localizedb.execute(selectquery);
	while(localizedb.getSelectData() != "") {
		resultstring = localizedb.getSelectData();
	}
	return resultstring;
}


std::string gLocalization::localizeWord(std:: string word) {
	gLogi("GameCanvas") << "Response:" << localizedb.getSelectData();

	selectquery = "SELECT "+columnList[currentlanguage]+" from WORDS where Key = '"+word+"'";
	//selectquery = "SELECT En from WORDS where Key = '"+word+"'";
	localizedb.execute(selectquery);
	//localizationdb.close();
	localizedword = localizedb.getSelectData();


	return localizedword;

}

