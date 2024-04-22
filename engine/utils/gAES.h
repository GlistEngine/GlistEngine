/*
 * gAES.h
 *
 *  Created on: 2 Sub 2022
 *      Author: Oguz
 */

#ifndef UTILS_GAES_H_
#define UTILS_GAES_H_

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/md5.h>
#include "gObject.h"


class gAES : public gObject {
public:
    gAES();
    virtual ~gAES();

    void initialize(unsigned char* initKey, unsigned char* initVector);

    std::string encrypt(std::string plainText); //AES-256
    std::string decrypt(std::string cipherText); //AES-256

    static std::string encodeMD5(std::string plainText); //MD5

private:
    unsigned char *initkey, *initvector;
    void handleErrors(void);
    void clean();
};
#endif /* UTILS_GAES_H_ */
