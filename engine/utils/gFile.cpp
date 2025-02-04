/*
 * gFile.cpp
 *
 *  Created on: Mar 17, 2021
 *      Author: noyan
 */

#include "gFile.h"

fs::copy_options gFile::copyOptions[4] = {fs::copy_options::none, fs::copy_options::skip_existing, fs::copy_options::overwrite_existing, fs::copy_options::recursive};

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
	stream.write(newBytes.data(), newBytes.size());

	if(mode == FILEMODE_READWRITE || mode == FILEMODE_APPEND) readFile();
}

void gFile::write(const char* bytes, size_t length) {
	stream.clear();
	stream.write(bytes, length);

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
	// Seek to the end to find the size of the file.
	stream.seekg(0, std::ios::end);
	size = stream.tellg();
	stream.seekg(0, std::ios::beg);  // Go back to the beginning of the file.

	// Reserve the size of the string to prevent reallocations.
	std::string filestr;
	filestr.resize(size);

	// Read the entire file into the string.
	stream.read(&filestr[0], size);

	// Remove all carriage return characters
	filestr.erase(std::remove(filestr.begin(), filestr.end(), '\r'), filestr.end());

	// Move the modified string into a vector<char>
	bytes = std::vector<char>(filestr.begin(), filestr.end());
	size = bytes.size();

	// Clear any flags and rewind if needed to read again later.
	stream.clear();
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

void gFile::copy(const std::string& fromFullPath, const std::string& toFullPath, int copyOption) {
	fs::copy(fromFullPath, toFullPath, copyOptions[copyOption]);
}

bool gFile::copyFile(const std::string& fromFullPath, const std::string& toFullPath, int copyOption) {
	return fs::copy_file(fromFullPath, toFullPath, copyOptions[copyOption]);
}

void gFile::copySymlink(const std::string& existingSymlinkFullPath, const std::string& newSymlinkFullPath) {
	fs::copy_symlink(existingSymlinkFullPath, newSymlinkFullPath);
}

bool gFile::createDirectory(const std::string& fullPath) {
	return fs::create_directory(fullPath);
}

void gFile::createDirectorySymlink(const std::string& toFullPath, const std::string& symlinkFullPath) {
	fs::create_directory_symlink(toFullPath, symlinkFullPath);
}

void gFile::createSymlink(const std::string& toFullPath, const std::string& symlinkFullPath) {
	fs::create_symlink(toFullPath, symlinkFullPath);
}

bool gFile::isEmpty(const std::string& fullPath) {
	return fs::is_empty(fullPath);
}

bool gFile::isEquivalent(const std::string& fullPath1, const std::string& fullPath2) {
	return fs::equivalent(fullPath1, fullPath2);
}

bool gFile::isSymlink(const std::string& fullPath) {
	return fs::is_symlink(fullPath);
}

bool gFile::remove(const std::string& fullPath) {
	return fs::remove(fullPath);
}

bool gFile::removeAll(const std::string& fullPath) {
	return fs::remove_all(fullPath);
}

void gFile::rename(const std::string& fromFullPath, const std::string& toFullPath) {
	fs::rename(fromFullPath, toFullPath);
}

std::vector<std::string> gFile::getDirectoryContent(const std::string& fullPath) {
	std::vector<std::string> content;
	for (const auto & entry : fs::directory_iterator(fullPath))
	        content.push_back(entry.path().stem().string());
	return content;
}

