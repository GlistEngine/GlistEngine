/*
 * gHttpFile.h
 *
 *  Created on: 10 Tem 2021
 *      Author: berka
 */

#ifndef UTILS_GHTTPFILE_H_
#define UTILS_GHTTPFILE_H_

#include <curl/curl.h>
#include <stdio.h>
#include <iostream>
#include <string>

#include "gFile.h"
#include "gObject.h"

class gHttpFile: public gObject {
public:

	gHttpFile();
	virtual ~gHttpFile();

	static size_t writeCallBack(char *contents, size_t size, size_t nmemb, void *userp);
	void load(std::string url);
	std::string getUrl();
	void save(std::string filepath);
	std::string getHtml();

private:
	std::string filepath;
	std::string url;
	gFile file;
	std::string curlbuffer;
};

#endif /* UTILS_GHTTPFILE_H_ */
