#include <exception>
#include <string>

class FileNotExistError : public std::exception {
public:
    FileNotExistError(const std::string &detail = std::string(""));
    virtual ~FileNotExistError();
    virtual const char *what() const noexcept override;

protected:
    std::string detail;
};

class FileTypeError : public std::exception {
public:
    FileTypeError(const std::string &detail = std::string(""));
    virtual ~FileTypeError();
    virtual const char *what() const noexcept override;

protected:
    std::string detail;
};

class DirectoryNotExistError : public std::exception {
public:
    DirectoryNotExistError(const std::string &detail = std::string(""));
    virtual ~DirectoryNotExistError();
    virtual const char *what() const noexcept override;

protected:
    std::string detail;
};