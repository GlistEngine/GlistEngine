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
	/*
	 * Metod for translating words to current language it will return string
	 * @ word Sending string for translate
	 */
	std::string localizeWord(std::string word);

	/*
	 * Load database for localization translate
	 *
	 * @database Sends database name as string for load and make ready to use for language translates
	 *
	 */
	void loadDatabase(std::string database);

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
	std::vector<std::string> columnList;
	//Variable which will contain a database
	gDatabase localizedb;
	//variables for translatint words
	int currentlanguage;
	std::string selectquery;
	std::string localizedword;
	std::string backupString;
	bool starttowrite;
	//string for result data
	std::string resultstring;


};

#endif /* UTILS_GLOCALIZATION_H_ */
