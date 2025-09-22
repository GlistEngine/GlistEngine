/*
 * gLocalization.h
 *
 *  Created on: 6 Tem 2021
 *      Author: Admin
 */

#ifndef UTILS_GLOCALIZATION_H_
#define UTILS_GLOCALIZATION_H_

#include <vector>
#include <string>
#include "gDatabase.h"

/**
 * Database-driven localization system for multi-language text translation.
 * Expects database table with "Key" column and language columns.
 */
class gLocalization {
public:
	gLocalization();
	virtual ~gLocalization();

	/**
     * Load localization database and parse available language columns
     * @param database Database file path
     * @param tableName Table containing translations (default: "WORDS")
     */
	void loadDatabase(const std::string& database, const std::string& tableName = "WORDS");

	/**
     * Translate word using current language setting
     * @param word Key to look up in database
     * @return Translated text, or original word if not found
     */
	std::string localizeWord(const std::string& word);

	/**
     * Set active language by column index (0-based, excluding "Key" column)
     * @param languageId Language index (bounds-checked)
     */
	void setCurrentLanguage(int languageId);

	/** @return Current language column name, empty if invalid */
	std::string getCurrentLanguage() const;

	/** @return Available language columns (excluding "Key") */
	const std::vector<std::string>& getAvailableLanguages() const;

	int getLangId() const;

	std::string getTableInfo();
	std::string getColumnData(const std::string& columnname);
	const std::vector<std::string>& getColumnList() const;
private:
	void parseColumnNames(const std::string& tableinfo);
	bool isValidLanguageId(int languageId) const;

	std::vector<std::string> columnlist;
	std::vector<std::string> availablelanguages;
	gDatabase localizedb;
	int currentlanguage;
	std::string tablename;
};

#endif /* UTILS_GLOCALIZATION_H_ */
