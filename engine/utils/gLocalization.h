/*
 * gLocalization.h
 *
 *  Created on: 6 Tem 2021
 *      Author: Admin
 */
#include <locale>
#include "gDatabase.h"

#ifndef UTILS_GLOCALIZATION_H_
#define UTILS_GLOCALIZATION_H_

class gLocalization {
public:
	gLocalization();
	virtual ~gLocalization();


	/*
	 * Load database for localization translate
	 *
	 * @database Sends database name as string for load and make ready to use for language translates
	 * @tableName Sends table name as string for set and make ready to use for other functions and language translates
	 */
	void loadDatabase(std::string database, std::string tableName = "WORDS");

	/*
	 * Metod for translating words to current language it will return string
	 * @ word Sending string for translate
	 */
	std::string localizeWord(std::string word);


	/*
	 * Allows setting current languages
	 *
	 * @languageId Sending id for setting which language will be use for translate
	 *
	 * @param languageId variable for setting language
	 */
	void setCurrentLanguage(int languageId);
	std::string getTableInfo();
	std::string getColumnData(std::string columnname);
	std::vector<std::string> getColumnList();

	/*
	 * Gives which language used
	 *
	 * @return language as string
	 */
	std::string getCurrentLanguage();

	/*
	 * Gives which language id is set
	 *
	 * @return return languageId as int
	 */
	int getLangId();

	int sayi;
private:
	//Add column names of database to columnList vector
	void columnLister(std::string tableinfo);
	std::vector<std::string> columnlist;
	//Variable which will contain a database
	gDatabase localizedb;
	//variables for translatint words
	int currentlanguage;
	std::string selectquery;
	std::string localizedword;
	std::string backupstring;
	bool starttowrite;
	//string for result data
	std::string resultstring;
	std::string tablename;


};

#endif /* UTILS_GLOCALIZATION_H_ */
