/*
 * gFile.cpp
 *
 *  Created on: Mar 17, 2021
 *      Author: noyan
 */

#include "gFile.h"

#ifdef WIN32
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #include <windows.h>
#endif

#ifdef WIN32
static std::wstring utf8ToWide(const std::string& s) {
    if (s.empty()) return std::wstring();

    int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                   s.c_str(), -1, nullptr, 0);

    UINT cp = CP_UTF8;
    if (wlen == 0) {
        cp = CP_ACP;
        wlen = MultiByteToWideChar(cp, 0, s.c_str(), -1, nullptr, 0);
        if (wlen == 0) return std::wstring();
    }

    std::wstring ws;
    ws.resize(wlen);

    MultiByteToWideChar(cp, 0, s.c_str(), -1, &ws[0], wlen);

    if (!ws.empty() && ws.back() == L'\0') ws.pop_back();
    return ws;
}

static fs::path toFsPath(const std::string& s) {
    return fs::path(utf8ToWide(s));
}
#else


static fs::path toFsPath(const std::string& s) {
    return fs::path(s);
}
#endif

fs::copy_options gFile::copyOptions[4] = {fs::copy_options::none, fs::copy_options::skip_existing, fs::copy_options::overwrite_existing, fs::copy_options::recursive};

gFile::gFile() : mode(FILEMODE_READONLY), binary(true), size(0) {}

gFile::gFile(const std::string& fullPath, int fileMode, bool isBinary) {
	load(fullPath, fileMode, isBinary);
}

gFile::~gFile() {}

bool gFile::load(const std::string& fullPath, int fileMode, bool isBinary) {
#ifdef _WIN32
    path = fs::path(utf8ToWide(fullPath));   // <-- kritik
#else
    path = fs::path(fullPath);
#endif
    mode = fileMode;
    binary = isBinary;
    size = 0;
    return open();
}

bool gFile::loadFile(const std::string& filePath, int fileMode, bool isBinary) {
#ifdef _WIN32
    std::string p = gGetFilesDir() + filePath;
    path = fs::path(utf8ToWide(p));          // <-- kritik
#else
    path = fs::path(gGetFilesDir() + filePath);
#endif
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

    close();

    std::ios_base::openmode m = binarymode;

    switch(mode) {
    case FILEMODE_READONLY:  m |= std::ios::in; break;
    case FILEMODE_WRITEONLY: m |= std::ios::out; break;
    case FILEMODE_READWRITE: m |= (std::ios::in | std::ios::out); break;
    case FILEMODE_APPEND:    m |= (std::ios::in | std::ios::out | std::ios::app); break;
    }

    stream.open(path, m);

    if(!stream.is_open() || !stream.good()) {
        gLoge("gFile") << "Error opening file " << path.u8string();
#ifdef _WIN32
    gLoge("gFile") << "Error opening file (native): " << path.string();
    gLoge("gFile") << "Error opening file (u8)    : " << path.u8string();
#else
    gLoge("gFile") << "Error opening file: " << path.string();
#endif

        return false;
    }

    // SADECE open baþarýlýysa oku
    if(mode == FILEMODE_READONLY || mode == FILEMODE_READWRITE || mode == FILEMODE_APPEND) {
        readFile();
    }

    return true;
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
	return doesFileExist(path.u8string());
}

bool gFile::isFile() {
	return isFile(path.u8string());
}

bool gFile::isLink() {
	return isLink(path.u8string());
}

bool gFile::isDirectory() {
	return isDirectory(path.u8string());
}

bool gFile::isDevice() {
	return isDevice(path);
}

bool gFile::isOpen() {
	return stream.is_open();
}

void gFile::readFile() {
    if(!stream.good()) {
        size = 0;
        bytes.clear();
        return;
    }

	// Seek to the end to find the size of the file.
	stream.seekg(0, std::ios::end);
	size = stream.tellg();
	stream.seekg(0, std::ios::beg);  // Go back to the beginning of the file.

	if (binary) {
		bytes = std::vector<char>(size);
		stream.read(bytes.data(), size);
	} else { // Replace carriage return characters in text files
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
	}

	// Clear any flags and rewind if needed to read again later.
	stream.clear();
}


// ----- Static functions -----

bool gFile::doesFileExist(const std::string& fullPath) {
    if(fullPath.empty()) return false;
    std::error_code ec;
	#ifdef _WIN32
		fs::path p(utf8ToWide(fullPath));
		bool ex = fs::exists(p, ec);
	#else
		bool ex = fs::exists(fullPath, ec);
	#endif
		if(ec) {
			gLoge("gFile") << "exists failed: " << ec.message();
			gLoge("gFile") << "path: " << fullPath;
			return false;
		}
		
    return ex;
}

bool gFile::doesFileExistInAssets(const std::string& filePath) {
	return doesFileExist(gGetFilesDir() + filePath);
}

bool gFile::isFile(const std::string& fullPath) {
    std::error_code ec;
    return fs::is_regular_file(toFsPath(fullPath), ec);
}

bool gFile::isFileInAssets(const std::string& filePath) {
	return isFile(gGetFilesDir() + filePath);
}

bool gFile::isLink(const std::string& fullPath) {
    std::error_code ec;
    return fs::is_symlink(toFsPath(fullPath), ec);
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
    std::error_code ec;
    return fs::is_directory(toFsPath(fullPath), ec);
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
    fs::copy(toFsPath(fromFullPath), toFsPath(toFullPath), copyOptions[copyOption]);
}

bool gFile::copyFile(const std::string& fromFullPath, const std::string& toFullPath, int copyOption) {
    std::error_code ec;
    bool ok = fs::copy_file(toFsPath(fromFullPath), toFsPath(toFullPath), copyOptions[copyOption], ec);
    if(ec) {
        gLoge("gFile") << "copyFile failed: " << ec.message();
        gLoge("gFile") << "from: " << fromFullPath;
        gLoge("gFile") << "to  : " << toFullPath;
    }
    return ok && !ec;
}

void gFile::copySymlink(const std::string& existingSymlinkFullPath, const std::string& newSymlinkFullPath) {
	fs::copy_symlink(existingSymlinkFullPath, newSymlinkFullPath);
}

bool gFile::createDirectory(const std::string& fullPath) {
    if(fullPath.empty()) return false;
    std::error_code ec;
	
	#ifdef _WIN32
		fs::path p(utf8ToWide(fullPath));
		bool ok = fs::create_directories(p, ec);
		if(!ec) return ok || fs::exists(p);
	#else
		bool ok = fs::create_directories(fullPath, ec);
		if(!ec) return ok || fs::exists(fullPath);
	#endif
	
    gLoge("gFile") << "createDirectory failed: " << ec.message();
    gLoge("gFile") << "path: " << fullPath;
    return false;
}

void gFile::createDirectorySymlink(const std::string& toFullPath, const std::string& symlinkFullPath) {
	fs::create_directory_symlink(toFullPath, symlinkFullPath);
}

void gFile::createSymlink(const std::string& toFullPath, const std::string& symlinkFullPath) {
	fs::create_symlink(toFullPath, symlinkFullPath);
}

bool gFile::isEmpty(const std::string& fullPath) {
	return fs::is_empty(toFsPath(fullPath));
}

bool gFile::isEquivalent(const std::string& fullPath1, const std::string& fullPath2) {
	return fs::equivalent(toFsPath(fullPath1), toFsPath(fullPath2));
}

bool gFile::isSymlink(const std::string& fullPath) {
	return fs::is_symlink(toFsPath(fullPath));
}

bool gFile::remove(const std::string& fullPath) {
	return fs::remove(toFsPath(fullPath));
}

bool gFile::removeAll(const std::string& fullPath) {
	return fs::remove_all(toFsPath(fullPath));
}

void gFile::rename(const std::string& fromFullPath, const std::string& toFullPath) {
    fs::rename(toFsPath(fromFullPath), toFsPath(toFullPath));
}

std::vector<std::string> gFile::getDirectoryContent(const std::string& fullPath) {
	std::vector<std::string> content;
	
	for (const auto & entry : fs::directory_iterator(toFsPath(fullPath)))
	        content.push_back(entry.path().stem().string());
	return content;
}

std::string gFile::normalizePathUtf8(const std::string& p) {
	#ifdef _WIN32
		return toFsPath(p).u8string();   // ACP/UTF8 karýþsa bile fs::path üzerinden normalize edip UTF-8 döndürür
	#else
		return p;
	#endif
}

