#include "exception.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/file_status.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>

void for_each(
    const std::function<void(const std::string                     &dir,
                             const std::tuple<std::string, size_t> &i)> &func,
    boost::property_tree::basic_ptree<std::string,
                                      std::tuple<std::string, size_t>>
                pt,
    std::string prefix = "") {
    if (pt.size() == 0) {
        auto data = pt.get_value<std::tuple<std::string, size_t>>();
        func(prefix, data);
        // std::cout << prefix << " -> " << std::get<0>(data) << ": "
        //           << std::get<1>(data) << "bytes." << std::endl;
        return;
    }
    // else
    for (auto iter = pt.begin(); iter != pt.end(); ++iter) {
        for_each(func, iter->second, prefix + "/" + iter->first);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        return 1;
    }
    std::string srcPath, destPath;
    if (std::string(argv[1]) == std::string("-o")) {
        srcPath  = std::string(argv[3]);
        destPath = std::string(argv[2]);
    }
    else if (std::string(argv[2]) == std::string("-o")) {
        srcPath  = std::string(argv[1]);
        destPath = std::string(argv[3]);
    }
    else {
        return 2;
    }

    boost::filesystem::path filenamePrefix =
        boost::filesystem::system_complete(boost::filesystem::path(srcPath))
            .branch_path();
    // if (boost::filesystem::path(srcPath).is_complete()) {
    //     filenamePrefix = boost::filesystem::path(srcPath).branch_path();
    // }
    // else {
    //     filenamePrefix = (boost::filesystem::current_path() / ;
    // }

    std::ifstream               in(srcPath);
    std::stringstream           outBuffer;
    std::ofstream               out(destPath, std::ios::ate | std::ios::out);
    boost::property_tree::ptree in_ptree;
    boost::property_tree::basic_ptree<std::string,
                                      std::tuple<std::string, size_t>>
        out_ptree; // std::tuple<string, string, size_t>: filename, size
    boost::property_tree::xml_parser::read_xml(
        in, in_ptree,
        boost::property_tree::xml_parser::no_comments |
            boost::property_tree::xml_parser::trim_whitespace);

    for (auto i = in_ptree.begin(); i != in_ptree.end(); ++i) {
        if (i->first == std::string("resource")) {
            std::string id =
                i->second.get<std::string>("<xmlattr>.id", "Resource");
            for (auto j = i->second.begin(); j != i->second.end(); ++j) {
                if (j->first == std::string("prefix")) {
                    std::string prefix =
                        j->second.get<std::string>("<xmlattr>.dir", ".");
                    for (auto k = j->second.begin(); k != j->second.end();
                         ++k) {
                        if (k->first == std::string("file")) {
                            // try to add a file
                            std::string filename, filenameAlias;
                            filename      = k->second.get_value<std::string>();
                            filenameAlias = k->second.get<std::string>(
                                "<xmlattr>.alias",
                                boost::filesystem::path(filename)
                                    .filename()
                                    .string());
                            filename = filenamePrefix.string() + "/" + filename;
                            if (!boost::filesystem::exists(filename)) {
                                throw FileNotExistError(
                                    std::string("File doesn't exists: ") +
                                    (boost::filesystem::path(prefix) /
                                     boost::filesystem::path(filename))
                                        .lexically_normal()
                                        .string());
                            }
                            auto status = boost::filesystem::status(filename);
                            switch (status.type()) {
                            case boost::filesystem::file_type::regular_file: {
                                size_t fileSize =
                                    boost::filesystem::file_size(filename);
                                std::string dir =
                                    boost::filesystem::path(prefix + "/" +
                                                            filenameAlias)
                                        .lexically_normal()
                                        .string();
                                out_ptree.put(
                                    boost::property_tree::path(dir, '/'),
                                    std::tuple<std::string, size_t>(filename,
                                                                    fileSize));
                                break;
                            }
                            defualt : {
                                throw FileTypeError(
                                    std::string("File type ERROR: ") +
                                    boost::filesystem::path(filename)
                                        .lexically_normal()
                                        .string() +
                                    std::string(
                                        "\nFile should be a regular file."));
                                break;
                            }
                            }
                        }
                        else if (k->first == std::string("directory")) {
                            throw std::exception();
                        }
                    }
                }
            }
            out << "#include <boost/property_tree/ptree.hpp>\n";
            out << "namespace SResourceData {\n";
            out << "    namespace " << id << " {\n";
            out << "        const char data[] = {";
            for_each(
                [&out](const std::string                     &dir,
                       const std::tuple<std::string, size_t> &data) -> void {
                    static int    cnt = 8;
                    std::ifstream f(std::get<0>(data));
                    char          c;
                    for (int i = 0; i < std::get<1>(data); ++i) {
                        if (cnt == 8) {
                            out << "\n           "; // 2 tabs and 3 spaces
                            cnt = 0;
                        }
                        f.read(&c, 1);
                        out << " 0x" << std::setw(2) << std::setfill('0')
                            << std::hex << (int)c << ',';
                        out << std::setw(0) << std::setfill(' ') << std::dec;
                        ++cnt;
                    }
                    f.close();
                },
                out_ptree);
            out << "0\n";
            out << "        };\n";
            out << "        boost::property_tree::basic_ptree<std::string, "
                   "std::pair<const char *, int>> pt;\n";
            out << "        class Dummy {\n";
            out << "        public:\n";
            out << "            Dummy() {\n";
            for_each(
                [&out](const std::string                     &dir,
                       const std::tuple<std::string, size_t> &data) -> void {
                    static int fileSizeTotal = 0;
                    out << "                "; // 4 tabs
                    out << "pt.put(boost::property_tree::path(\":" << dir
                        << "\", \'/\'), std::make_pair<const char *, "
                           "int>(&data["
                        << fileSizeTotal << "], " << std::get<1>(data)
                        << "));\n";
                    fileSizeTotal += std::get<1>(data);
                },
                out_ptree);
            out << "            }\n";
            out << "        } dummy;\n";
            out << "    }\n";
            out << "}\n";
        }
    }
    return 0;
}