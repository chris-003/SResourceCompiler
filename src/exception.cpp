#include "exception.h"

FileNotExistError::FileNotExistError(const std::string &detail) {
    this->detail = detail;
}

FileNotExistError::~FileNotExistError() {
}

const char *FileNotExistError::what() const noexcept {
    return detail.data();
}

FileTypeError::FileTypeError(const std::string &detail) {
    this->detail = detail;
}

FileTypeError::~FileTypeError() {
}

const char *FileTypeError::what() const noexcept {
    return detail.data();
}

DirectoryNotExistError::DirectoryNotExistError(const std::string &detail) {
    this->detail = detail;
}

DirectoryNotExistError::~DirectoryNotExistError() {
}

const char *DirectoryNotExistError::what() const noexcept {
    return detail.data();
}