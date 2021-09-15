/*
 * gLocalization.h
 *
 *  Created on: 6 Tem 2021
 *      Author: Admin
 */
#include <locale>
#include <gDatabase.h>

#ifndef UTILS_GLOCALIZATION_H_
#define UTILS_GLOCALIZATION_H_

class gLocalization {
public:
	gLocalization();
	virtual ~gLocalization();
	//Metodlar
	std::string localizeWord(std::string word);
	void loadDatabase(std::string database);

	void setCurrentLanguage(int languageId);
	std::string getTableInfo();
	std::string getColumnData(std::string columnname);
	std::string getCurrentLanguage();
	int getLangId();
	int sayi;
private:
	//Add column names of database to columnList vector
	void columnLister(std::string tableinfo);
	std::vector<std::string> columnList;
	//Database'i okuyacak nesne
	gDatabase localizedb;
	//çevrilecek metini iþleyecek olan deðiþkenler
	int currentlanguage;
	std::string selectquery;
	std::string localizedword;
	std::string backupString;
	bool starttowrite;
	//string for result data
	std::string resultstring;


};

#endif /* UTILS_GLOCALIZATION_H_ */
