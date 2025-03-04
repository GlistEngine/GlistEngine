/*
 * Copyright (C) 2016 Nitra Games Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UTILS_GFILE_H_
#define UTILS_GFILE_H_

//#include <fstream>
#include <ios>
#include <iostream>
#include "ghc/filesystem.hpp"
namespace fs {
using namespace ghc::filesystem;
using ifstream = ghc::filesystem::ifstream;
using ofstream = ghc::filesystem::ofstream;
using fstream = ghc::filesystem::fstream;
}
#include "gObject.h"


class gFile : public gObject {
public:
	enum Mode {
		FILEMODE_READONLY,
		FILEMODE_WRITEONLY,
		FILEMODE_READWRITE,
		FILEMODE_APPEND
	};

	enum CopyOptions {
		NONE,
		SKIP_EXISTING,
		OVERWRITE_EXISTING,
		RECURSIVE
	};

	gFile();
	virtual ~gFile();

	bool load(const std::string& fullPath, int fileMode = FILEMODE_READONLY, bool isBinary = true);
	bool loadFile(const std::string& filePath, int fileMode = FILEMODE_READONLY, bool isBinary = true);
	void close();

	fs::path getPath();
	int getMode();
	bool isBinary();

	void write(const std::string& content);
	void write(std::vector<char> newBytes);
	void write(const char* bytes, size_t length);

	std::vector<char> getBytes();

	const std::vector<char>& getBytesConst() {
		return bytes;
	}

	std::string getText();
	int getSize();

	std::string getFilename();
	std::string getDirectory();

	bool exists();
	bool isFile();
	bool isLink();
	bool isDirectory();
	bool isDevice();
	bool isOpen();

	static bool doesFileExist(const std::string& fullPath);
	static bool doesFileExistInAssets(const std::string& filePath);
	static bool isFile(const std::string& fullPath);
	static bool isFileInAssets(const std::string& filePath);
	static bool isLink(const std::string& fullPath);
	static bool isLinkInAssets(const std::string& filePath);
	static bool isDirectory(const std::string& fullPath);
	static bool isDirectoryInAssets(const std::string& filePath);
	static bool isDevice(const fs::path& path);
	static std::string getFilename(const fs::path& path);
	static std::string getDirectory(const fs::path& path);
	static std::string addComplementarySlashIfNeeded(const fs::path& filePath);

	//copyOptions can be one of;
	//gFile::CopyOptions::NONE, gFile::CopyOptions::SKIP_EXISTING, gFile::CopyOptions::OVERWRITE_EXISTING
	static void copy(const std::string& fromFullPath, const std::string& toFullPath, int copyOption = CopyOptions::NONE);
	static bool copyFile(const std::string& fromFullPath, const std::string& toFullPath, int copyOption = CopyOptions::NONE);
	static void copySymlink(const std::string& existingSymlinkFullPath, const std::string& newSymlinkFullPath);
	static bool createDirectory(const std::string& fullPath);
	static void createDirectorySymlink(const std::string& toFullPath, const std::string& symlinkFullPath);
	static void createSymlink(const std::string& toFullPath, const std::string& symlinkFullPath);
	static bool isEmpty(const std::string& fullPath);
	static bool isEquivalent(const std::string& fullPath1, const std::string& fullPath2);
	static bool isSymlink(const std::string& fullPath);
	static bool remove(const std::string& fullPath);
	static bool removeAll(const std::string& fullPath);
	static void rename(const std::string& fromFullPath, const std::string& toFullPath);
	static std::vector<std::string> getDirectoryContent(const std::string& fullPath);

private:
	fs::path path;
	int mode;
	bool binary;

	std::vector<char> bytes;
	int size;

	fs::fstream stream;
	bool open();
	bool openStream(int fileMode, bool isBinary);
	void readFile();

	static fs::copy_options copyOptions[4];
};

#endif /* UTILS_GFILE_H_ */
