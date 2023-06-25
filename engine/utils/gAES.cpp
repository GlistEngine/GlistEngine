/*
 * gAES.cpp
 *
 *  Created on: 2 Sub 2022
 *      Author: Oguz
 */

#include "gAES.h"

#if(ANDROID)
// todo
#else
gAES::gAES() {
	unsigned char empty[] = { 0 };
	initkey = empty;
	initvector = empty;
}

gAES::~gAES() {
}

void gAES::initialize(unsigned char* initKey, unsigned char* initVector) {
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	CONF_modules_load(NULL, NULL, 0);
	initkey = initKey;
	initvector = initVector;
}

std::string gAES::encrypt(std::string plainText) {
	unsigned char ciphertext[plainText.length() * 3];
	int ciphertext_len;
	int len;
	EVP_CIPHER_CTX *ctx;

	/* Create and initialise the context */
	if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

	/* Initialise the encryption operation. IMPORTANT - ensure you use a key
	 * and IV size appropriate for your cipher
	 */
	if(!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, initkey, initvector)) handleErrors();

	/* Provide the message to be encrypted, and obtain the encrypted output.
	 * EVP_EncryptUpdate can be called multiple times if necessary
	 */
	if(!EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)plainText.c_str(), (int)plainText.length())) handleErrors();
	ciphertext_len = len;

	/* Finalise the encryption. Further ciphertext bytes may be written at
	 * this stage.
	 */
	if(!EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
	ciphertext_len += len;

	/* Clean up */
	EVP_CIPHER_CTX_free(ctx);

	std::string str(reinterpret_cast<char*>(ciphertext));
	clean();
	return str.substr(0, ciphertext_len);
}

std::string gAES::decrypt(std::string cipherText) {
	unsigned char plaintext[cipherText.length() * 3];
	int len;
	int plaintext_len;
	EVP_CIPHER_CTX *ctx;
	if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

	if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, initkey, initvector)) handleErrors();

	if(!EVP_DecryptUpdate(ctx, plaintext, &len, (unsigned char*)cipherText.c_str(), (int)cipherText.length())) handleErrors();
	plaintext_len = len;

	if(!EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleErrors();
	plaintext_len += len;

	EVP_CIPHER_CTX_free(ctx);
	std::string str(reinterpret_cast<char*>(plaintext));
	clean();
	return str.substr(0, plaintext_len);
}

std::string gAES::encodeMD5(std::string plainText) {
	unsigned char digest[MD5_DIGEST_LENGTH];
	MD5((unsigned char*)plainText.c_str(), (int)plainText.length(), digest);
	char mdString[33];

	for(int i = 0; i < 16; i++)
		sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);

	std::string hashText(reinterpret_cast<char*>(mdString));
	return hashText;
}

void gAES::clean() {
	/* Clean up */
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
	ERR_free_strings();
}

void gAES::handleErrors(void) {
	ERR_print_errors_fp(stderr);
	abort();
}
#endif

