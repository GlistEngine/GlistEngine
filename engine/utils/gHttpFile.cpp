/*
 * gHttpFile.cpp
 *
 *  Created on: 10 Tem 2021
 *      Author: berka
 */

#include "gHttpFile.h"

gHttpFile::gHttpFile() {
}

gHttpFile::~gHttpFile() {
}

size_t gHttpFile::writeCallBack(char *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void gHttpFile::load(std::string url) {
	this->url = url;
	loadHtml();
}

std::string gHttpFile::getUrl() {
	return url;
}

std::string gHttpFile::getHtml() {
	return html;
}

void gHttpFile::save(std::string filePath, bool isBinary) {
	file.load(filePath, 1, isBinary);
	file.write(html);
	file.close();
}

void gHttpFile::loadHtml() {
	html = "";
	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    //  enable this command for seing verbose information. Useful for debugging and tracking the request.
	//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &gHttpFile::writeCallBack);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);

#ifdef SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

    	/* Perform the request, res will get the return code */
    	res = curl_easy_perform(curl);
    	/* Check for errors */
    	if(res != CURLE_OK)
    	fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    	/* always cleanup */
    	curl_easy_cleanup(curl);
	}

	curl_global_cleanup();
}
