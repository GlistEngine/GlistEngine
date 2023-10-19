/*
 * gFile.cpp
 *
 *  Created on: Mar 17, 2021
 *      Author: noyan
 */

#include "gFile.h"


gFile::gFile() : mode(FILEMODE_READONLY), binary(true), size(0) {}

gFile::~gFile() {}

bool gFile::load(const std::string& fullPath, int fileMode, bool isBinary) {
	path = fs::path(fullPath);
	mode = fileMode;
	binary = isBinary;
	size = 0;
	return open();
}

bool gFile::loadFile(const std::string& filePath, int fileMode, bool isBinary) {
	path = fs::path(gGetFilesDir() + filePath);
	mode = fileMode;
	binary = isBinary;
	size = 0;
	return open();
}

fs::path gFile::getPath() {
	return path;
}

int gFile::getMode() {
	return mode;
}

bool gFile::isBinary() {
	return binary;
}

void gFile::write(const std::string& content) {
	stream.clear();
	stream << content;

	if(mode == FILEMODE_READWRITE || mode == FILEMODE_APPEND) readFile();
}

void gFile::write(std::vector<char> newBytes) {
	stream.clear();
	for(int i = 0; i < newBytes.size(); i++) stream << newBytes[i];

	if(mode == FILEMODE_READWRITE || mode == FILEMODE_APPEND) readFile();
}

bool gFile::open() {
	close();
	return openStream(mode, binary);
}

void gFile::close() {
	if (stream.is_open()) stream.close();
}

bool gFile::openStream(int fileMode, bool isBinary) {
	mode = fileMode;
	binary = isBinary;
	std::ios_base::openmode binarymode = binary ? std::ios::binary : (std::ios_base::openmode)0;

	switch(mode) {
	case FILEMODE_READONLY:
		stream.open(path, std::ios::in | binarymode);
		readFile();
		break;
	case FILEMODE_WRITEONLY:
		stream.open(path, std::ios::out | binarymode);
		break;
	case FILEMODE_READWRITE:
		stream.open(path, std::ios::in | std::ios::out | binarymode);
		readFile();
		break;
	case FILEMODE_APPEND:
		stream.open(path, std::ios::in | std::ios::out | std::ios::app | binarymode);
		readFile();
		break;
	}

	if(!stream) {
        gLoge("gFile") << "Error opening file " << path;
		return -1;
	}

	return stream.good();
}

std::vector<char> gFile::getBytes() {
	return bytes;
}

std::string gFile::getText() {
	if(!size) return "";

	return std::string(bytes.data(), size);
}

int gFile::getSize() {
	return size;
}

std::string gFile::getFilename() {
	return getFilename(path);
}

std::string gFile::getDirectory() {
	return getDirectory(path);
}

bool gFile::exists() {
	return doesFileExist(path.string());
}

bool gFile::isFile() {
	return isFile(path.string());
}

bool gFile::isLink() {
	return isLink(path.string());
}

bool gFile::isDirectory() {
	return isDirectory(path.string());
}

bool gFile::isDevice() {
	return isDevice(path);
}

bool gFile::isOpen() {
	return stream.is_open();
}

void gFile::readFile() {
	stream.seekg(0, std::ios::beg);
	stream.ignore( std::numeric_limits<std::streamsize>::max() );
	size = stream.gcount();
	stream.clear();   //  Since ignore will have set eof.
	stream.seekg(0, std::ios::beg);

	bytes.clear();
	bytes.resize(size);
	stream.read(bytes.data(), size);
}


// ----- Static functions -----

bool gFile::doesFileExist(const std::string& fullPath) {
	if(fullPath.empty()){
		return false;
	}

	return fs::exists(fullPath);
}

bool gFile::doesFileExistInAssets(const std::string& filePath) {
	return doesFileExist(gGetFilesDir() + filePath);
}

bool gFile::isFile(const std::string& fullPath) {
	return fs::is_regular_file(fullPath);
}

bool gFile::isFileInAssets(const std::string& filePath) {
	return isFile(gGetFilesDir() + filePath);
}

bool gFile::isLink(const std::string& fullPath) {
	return fs::is_symlink(fullPath);
}

bool gFile::isLinkInAssets(const std::string& filePath) {
	return isLink(gGetFilesDir() + filePath);
}

bool gFile::isDevice(const fs::path& path) {
#ifdef WIN32
	return false;
#else
	return fs::is_block_file(fs::status(path));
#endif
}

bool gFile::isDirectory(const std::string& fullPath) {
	return fs::is_directory(fullPath);
}

bool gFile::isDirectoryInAssets(const std::string& filePath) {
	return isDirectory(gGetFilesDir() + filePath);
}

std::string gFile::getFilename(const fs::path& path) {
	return path.stem().string();
}

std::string gFile::getDirectory(const fs::path& path) {
	return addComplementarySlashIfNeeded(fs::path(path).parent_path().string());
}

std::string gFile::addComplementarySlashIfNeeded(const fs::path& path) {
    std::string fpath = fs::path(path).make_preferred().string();
	fs::path lastchar = fs::path("/").make_preferred();
	if(!fpath.empty()){
		if(gToStr(fpath.back()) != lastchar.string()){
			fpath = (fpath / lastchar).string();
		}
	}
	return fpath;
}


