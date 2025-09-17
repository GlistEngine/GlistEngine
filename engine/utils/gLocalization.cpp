/*
 * gLocalization.cpp
 *
 *  Created on: 6 Tem 2021
 *      Author: Admin
 */

#include "gLocalization.h"
#include <sstream>

gLocalization::gLocalization() : currentlanguage(0) {
}

gLocalization::~gLocalization() {
    localizedb.close();
}

void gLocalization::loadDatabase(const std::string& database, const std::string& tableName) {
    tablename = tableName;
    currentlanguage = 0;
    columnlist.clear();

    localizedb.loadDatabase(database);

    // Get table schema
    std::string selectquery = "SELECT sql FROM sqlite_master WHERE tbl_name = '" + tablename + "' AND type = 'table'";
    localizedb.execute(selectquery);
    std::string tableinfo = localizedb.getSelectData();

    parseColumnNames(tableinfo);
}

void gLocalization::parseColumnNames(const std::string& tableinfo) {
    columnlist.clear();
	availablelanguages.clear();

    // Find CREATE TABLE statement and extract column definitions
    size_t createpos = tableinfo.find("CREATE TABLE");
    if (createpos == std::string::npos) return;

    size_t openparen = tableinfo.find('(', createpos);
    size_t closeparen = tableinfo.rfind(')');

    if (openparen == std::string::npos || closeparen == std::string::npos || openparen >= closeparen) {
        return;
    }

    // Extract column definitions between parentheses
    std::string columndefs = tableinfo.substr(openparen + 1, closeparen - openparen - 1);

    // Split by commas (simplified - doesn't handle commas in constraints)
    std::stringstream ss(columndefs);
    std::string columndef;

    while (std::getline(ss, columndef, ',')) {
        // Trim whitespace
        columndef.erase(0, columndef.find_first_not_of(" \t\n\r"));
        columndef.erase(columndef.find_last_not_of(" \t\n\r") + 1);

        // Extract column name (first word, possibly quoted)
        std::string columnname;
        if (!columndef.empty()) {
            if (columndef[0] == '"') {
                // Quoted column name
                size_t endQuote = columndef.find('"', 1);
                if (endQuote != std::string::npos) {
                    columnname = columndef.substr(1, endQuote - 1);
                }
            } else {
                // Unquoted column name - take first word
                size_t spacePos = columndef.find(' ');
                columnname = columndef.substr(0, spacePos);
            }

            if (!columnname.empty()) {
                columnlist.push_back(columnname);
            }
        }
    }
	for (int i = 1; i < columnlist.size(); i++) {
		availablelanguages.push_back(columnlist[i]);
	}
}

std::string gLocalization::getTableInfo() {
    std::string selectquery = "SELECT sql FROM sqlite_master WHERE tbl_name = '" + tablename + "' AND type = 'table'";
    localizedb.execute(selectquery);
    return localizedb.getSelectData();
}

const std::vector<std::string>& gLocalization::getColumnList() const {
    return columnlist;
}

void gLocalization::setCurrentLanguage(int languageId) {
    if (isValidLanguageId(languageId)) {
    	gLogw("gLocalization") << "Changed current language to " << getCurrentLanguage() << " (id: " << languageId << ")" ;
        currentlanguage = languageId;
    } else {
    	std::string languages;
    	for (const std::string& name : availablelanguages) {
    		languages += name;
    		languages += ", ";
    	}
    	languages.erase(languages.length() - 2);
        gLogw("gLocalization") << "Invalid language id: " << languageId << ", available languages: " << languages << " (c: " << availablelanguages.size() << ")";
    }
}

bool gLocalization::isValidLanguageId(int languageId) const {
    // Assuming first column is "Key", so language columns start from index 1
    return languageId >= 0 && (languageId + 1) < static_cast<int>(columnlist.size());
}

std::string gLocalization::getCurrentLanguage() const {
    if (!isValidLanguageId(currentlanguage)) {
        return "";
    }
    return columnlist[currentlanguage + 1];
}

int gLocalization::getLangId() const {
    return currentlanguage;
}

const std::vector<std::string>& gLocalization::getAvailableLanguages() const {
	return availablelanguages;
}

std::string gLocalization::getColumnData(const std::string& columnname) {
    std::string selectquery = "SELECT " + columnname + " FROM " + tablename;
    localizedb.execute(selectquery);

    std::string resultstring;
    std::string data;
	do {
		data = localizedb.getSelectData();
		if (!data.empty()) {
			resultstring = data;
		}
	} while (!data.empty());

    return resultstring;
}

std::string gLocalization::localizeWord(const std::string& word) {
    if (!isValidLanguageId(currentlanguage)) {
        gLogw("gLocalization") << "Invalid language id: " << currentlanguage;
        return word; // Return original word as fallback
    }

    const std::string& languagecolumn = columnlist[currentlanguage + 1];
    //gLogi("gLocalization") << word << ", " << currentlanguage << ", " << languageColumn << ", " << tablename;

    std::string selectquery = "SELECT " + languagecolumn + " FROM " + tablename + " WHERE Key = '" + word + "'";
    localizedb.execute(selectquery);

    std::string result = localizedb.getSelectData();
    if (!result.empty()) {
        // Handle the "|" splitting if needed
        std::vector<std::string> parts = gSplitString(result, "|");
        if (parts.size() > 1) {
            return parts[1];
        }
        return result;
    }

	gLogw("gLocalization") << "Could not find word: " << word;
    return word; // Return original word if translation not found
}