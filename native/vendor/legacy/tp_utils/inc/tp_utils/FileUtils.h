#ifndef tp_utils_FileUtils_h
#define tp_utils_FileUtils_h

#include "tp_utils/Globals.h"

#include "tp_utils/JSONUtils.h"

#include <string>
#include <unordered_set>

namespace tp_utils
{

//##################################################################################################
[[nodiscard]]std::string TP_UTILS_EXPORT readTextFile(const std::string& filename);

//##################################################################################################
[[nodiscard]]std::string TP_UTILS_EXPORT readBinaryFile(const std::string& filename);

//##################################################################################################
//! Writes a string to a file
/*!
\param filename - The path to the file to write to
\param textOutput - The text to write out
\return True if the file was written, else false.
 */
bool TP_UTILS_EXPORT writeTextFile(const std::string& filename, const std::string& textOutput, TPAppend append=TPAppend::No);

//##################################################################################################
//! Writes a string to a file
/*!
\param filename - The path to the file to write to
\param textOutput - The text to write out
\return True if the file was written, else false.
 */
bool TP_UTILS_EXPORT writeBinaryFile(const std::string& filename, const std::string& binaryOutput);

//##################################################################################################
[[nodiscard]]JSON TP_UTILS_EXPORT readJSONFile(const std::string& filename);

//##################################################################################################
bool TP_UTILS_EXPORT writeJSONFile(const std::string& filename, const JSON& j, int indent = -1);

//##################################################################################################
bool TP_UTILS_EXPORT writePrettyJSONFile(const std::string& filename, const JSON& j);

//##################################################################################################
// Under windows Powershell has wget with https support
// since windows 8. Calling it from c++ code involces the Invoke-webrequest.
std::string TP_UTILS_EXPORT readFileFromURL(const std::string& url, const std::string& name, const std::string& rootPath);
    
//##################################################################################################
//!
/*!
\param path The directory to list files in.
\param extensions File extensions to search in the format "*.png".
\return A list of absolute paths.
*/
[[nodiscard]]std::vector<std::string> TP_UTILS_EXPORT listFiles(const std::string& path, const std::unordered_set<std::string>& extensions={});

//##################################################################################################
std::vector<std::string> listFilesOrdered(const std::string& path, const std::unordered_set<std::string>& extensions);

//##################################################################################################
//!
/*!
\param path The directory to list directories in.
\return A list of absolute paths.
*/
[[nodiscard]]std::vector<std::string> TP_UTILS_EXPORT listDirectories(const std::string& path);

//##################################################################################################
[[nodiscard]]int64_t TP_UTILS_EXPORT fileTimeMS(const std::string& path);

//##################################################################################################
[[nodiscard]]int64_t TP_UTILS_EXPORT fileAgeMS(const std::string& path);

//##################################################################################################
bool TP_UTILS_EXPORT copyFile(const std::string& pathFrom, const std::string& pathTo);

//##################################################################################################
bool TP_UTILS_EXPORT cp(const std::string& pathFrom, const std::string& pathTo, TPRecursive recursive);

//##################################################################################################
bool TP_UTILS_EXPORT mv(const std::string& pathFrom, const std::string& pathTo);

//##################################################################################################
bool TP_UTILS_EXPORT mkdir(const std::string& path, TPCreateFullPath createFullPath);

//##################################################################################################
bool TP_UTILS_EXPORT rm(const std::string& path, TPRecursive recursive);

//##################################################################################################
[[nodiscard]]bool TP_UTILS_EXPORT exists(const std::string& path);

//##################################################################################################
[[nodiscard]]size_t TP_UTILS_EXPORT fileSize(const std::string& path);

//##################################################################################################
bool TP_UTILS_EXPORT setCWD(const std::string& path);

//##################################################################################################
std::string TP_UTILS_EXPORT cwd();

//##################################################################################################
enum perms : unsigned
{
  owner_read = 0400 , // S_IRUSR, Read permission, owner
  owner_write = 0200, // S_IWUSR, Write permission, owner
  owner_exec = 0100 , // S_IXUSR, Execute/search permission, owner
  owner_all = 0700  , // S_IRWXU, Read, write, execute/search by owner

  group_read = 040  , // S_IRGRP, Read permission, group
  group_write = 020 , // S_IWGRP, Write permission, group
  group_exec = 010  , // S_IXGRP, Execute/search permission, group
  group_all = 070   , // S_IRWXG, Read, write, execute/search by group

  others_read = 04  , // S_IROTH, Read permission, others
  others_write = 02 , // S_IWOTH, Write permission, others
  others_exec = 01  , // S_IXOTH, Execute/search permission, others
  others_all = 07     // S_IRWXO, Read, write, execute/search by others
};

//##################################################################################################
bool TP_UTILS_EXPORT setPermissions(const std::string& path, unsigned permissions);

//##################################################################################################
inline bool chmod(const std::string& path, unsigned permissions)
{
  return setPermissions(path, permissions);
}


//##################################################################################################
[[nodiscard]]std::string TP_UTILS_EXPORT filename(const std::string& path);

//##################################################################################################
[[nodiscard]]std::string TP_UTILS_EXPORT removeExtension(const std::string& filename);

//##################################################################################################
[[nodiscard]]std::string TP_UTILS_EXPORT extension(const std::string& filename);

//##################################################################################################
[[nodiscard]]std::string TP_UTILS_EXPORT replaceExtension(const std::string& filename, const std::string& ext);

//##################################################################################################
[[nodiscard]]std::string TP_UTILS_EXPORT directoryName(const std::string& path);

//##################################################################################################
[[nodiscard]]std::string TP_UTILS_EXPORT pathAppend(const std::string& path, const std::string& part);

//##################################################################################################
[[nodiscard]]std::string TP_UTILS_EXPORT appendPaths(const std::vector<std::string>& paths);

//##################################################################################################
[[nodiscard]]std::string TP_UTILS_EXPORT pythonJsonPath(std::string path);

//##################################################################################################
[[nodiscard]]std::string TP_UTILS_EXPORT absolute(const std::string& path);

//##################################################################################################
extern std::vector<std::string> TP_UTILS_EXPORT (*listFilesCallback)(const std::string& path, const std::unordered_set<std::string>& extensions);
extern std::vector<std::string> TP_UTILS_EXPORT (*listDirectoriesCallback)(const std::string& path);
extern int64_t TP_UTILS_EXPORT (*fileTimeMSCallback)(const std::string& path);
extern int64_t TP_UTILS_EXPORT (*fileAgeMSCallback)(const std::string& path);
extern bool TP_UTILS_EXPORT (*copyFileCallback)(const std::string& pathFrom, const std::string& pathTo);
extern bool TP_UTILS_EXPORT (*cpCallback)(const std::string& pathFrom, const std::string& pathTo, TPRecursive recursive);
extern bool TP_UTILS_EXPORT (*mvCallback)(const std::string& pathFrom, const std::string& pathTo);
extern bool TP_UTILS_EXPORT (*mkdirCallback)(const std::string& path, TPCreateFullPath createFullPath);
extern bool TP_UTILS_EXPORT (*rmCallback)(const std::string& path, TPRecursive recursive);
extern bool TP_UTILS_EXPORT (*existsCallback)(const std::string& path);
extern std::string TP_UTILS_EXPORT (*absoluteCallback)(const std::string& path);
extern size_t TP_UTILS_EXPORT (*fileSizeCallback)(const std::string& path);
extern bool TP_UTILS_EXPORT (*setCWDCallback)(const std::string& path);
extern std::string TP_UTILS_EXPORT (*cwdCallback)();
extern bool TP_UTILS_EXPORT (*setPermissionsCallback)(const std::string& path, unsigned permissionsh);

#if __cplusplus >= 202002L
inline std::filesystem::path u8path(std::string const& filename)
{
    return std::filesystem::path(reinterpret_cast<const char8_t*>(filename.c_str()));
}

#else
inline std::filesystem::path u8path(std::string const& filename)
{
    return std::filesystem::u8path(filename);
}
#endif


}

#endif
