/*
 * gHttpFile.h
 *
 *  Created on: 10 Tem 2021
 *      Author: berka
 */

#ifndef UTILS_GHTTPFILE_H_
#define UTILS_GHTTPFILE_H_
#if(ANDROID)

#else
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
	struct ProgressData {
		double progresslength;
		double filelength;
	} prog;
	static int progressCallback(ProgressData *p, double totaltodownload, double downloaded, double totaltoupload, double uploaded);
	static size_t writeCallBack(char *contents, size_t size, size_t nmemb, void *userp);
	void load(std::string url);
	std::string getUrl();
	void save(std::string filepath, bool isBinary = false);
	std::string getHtml();
	double getProgressLength();
	double getFileLength();

  private:
	std::string filepath;
	std::string url;
	gFile file;
	std::string html;
	void loadHtml();
};
#endif

#endif /* UTILS_GHTTPFILE_H_ */
