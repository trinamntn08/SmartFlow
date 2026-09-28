#include "tp_utils/FileUtils.h"

#include <fstream>
#include <streambuf>
#include <filesystem> 
#include <cstdlib>

namespace tp_utils
{

//##################################################################################################
std::string TP_UTILS_EXPORT readTextFile(const std::string& filename)
{
  try
  {
    std::ifstream in(tp_utils::u8path(filename));
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  }
  catch(...)
  {
    return std::string();
  }
}

//##################################################################################################
std::string TP_UTILS_EXPORT readBinaryFile(const std::string& filename)
{
  try
  {
    std::ifstream in(tp_utils::u8path(filename), std::ios::binary | std::ios::ate);
    std::string results;
    auto size = in.tellg();
    if(size>0)
    {
      results.resize(size_t(size));

      in.seekg(0);
      int64_t read=0;
      while(read<size && !in.eof())
      {
        in.read(&results[size_t(read)], ptrdiff_t(size)-ptrdiff_t(read));
        read += int64_t(in.gcount());
      }

      if(size_t(read)!=results.size())
        results.resize(size_t(read));
    }
    return results;
  }
  catch(...)
  {
    return std::string();
  }
}


//##################################################################################################
bool TP_UTILS_EXPORT writeTextFile(const std::string& filename, const std::string& textOutput, TPAppend append)
{
  try
  {
    std::ofstream out;

    if(append == TPAppend::Yes)
      out.open(tp_utils::u8path(filename), std::ios_base::out | std::ios_base::app);
    else
      out.open(tp_utils::u8path(filename), std::ios_base::out | std::ios_base::trunc);

    out << textOutput;
    return true;
  }
  catch(...)
  {
    return false;
  }
}

//##################################################################################################
bool TP_UTILS_EXPORT writeBinaryFile(const std::string& filename, const std::string& binaryOutput)
{
  try
  {
    std::ofstream out(tp_utils::u8path(filename), std::ios::binary);

    out << binaryOutput;
    return true;
  }
  catch(...)
  {
    return false;
  }
}

//##################################################################################################
JSON TP_UTILS_EXPORT readJSONFile(const std::string& filename)
{
  try
  {
    std::string str = readTextFile(filename);
    return JSON::parse(str);
  }
  catch(...)
  {
    return JSON();
  }
}

//##################################################################################################
bool writeJSONFile(const std::string& filename, const JSON& j, int indent)
{
  try
  {
    std::string s = j.dump(indent);
    return writeTextFile(filename, s);
  }
  catch(...)
  {
    return false;
  }
}

//##################################################################################################
bool writePrettyJSONFile(const std::string& filename, const JSON& j)
{
  return writeJSONFile(filename, j, 2);
}

//##################################################################################################
std::string readFileFromURL(const std::string& url, const std::string& name, const std::string& rootPath)
{
  std::string filePath = rootPath + "/" + std::filesystem::path(name).filename().string();
#ifdef TP_WIN32
  std::string cmd = "powershell -Command \"$ProgressPreference='SilentlyContinue'; Invoke-WebRequest -Uri '" + url +
      "' -OutFile '" + filePath + "'\"";
  int res = system(cmd.c_str());
  if(0 != res)
  {
    return std::string();
  }
  return filePath;
#else
  int res = system((std::string("wget -O \"") + filePath + "\" \"" + url + "\"").c_str());
  if(0 != res)
    return std::string();

  return filePath;
#endif
}

//##################################################################################################
std::vector<std::string> (*listFilesCallback)(const std::string& path, const std::unordered_set<std::string>& extensions)=nullptr;
std::vector<std::string> (*listDirectoriesCallback)(const std::string& path)=nullptr;
int64_t (*fileTimeMSCallback)(const std::string& path)=nullptr;
int64_t (*fileAgeMSCallback)(const std::string& path)=nullptr;
bool (*copyFileCallback)(const std::string& pathFrom, const std::string& pathTo)=nullptr;
bool (*cpCallback)(const std::string& pathFrom, const std::string& pathTo, TPRecursive recursive)=nullptr;
bool (*mvCallback)(const std::string& pathFrom, const std::string& pathTo)=nullptr;
bool (*mkdirCallback)(const std::string& path, TPCreateFullPath createFullPath)=nullptr;
bool (*rmCallback)(const std::string& path, TPRecursive recursive)=nullptr;
bool (*existsCallback)(const std::string& path)=nullptr;
std::string TP_UTILS_EXPORT (*absoluteCallback)(const std::string& path)=nullptr;
size_t (*fileSizeCallback)(const std::string& path)=nullptr;
bool (*setCWDCallback)(const std::string& path)=nullptr;
std::string (*cwdCallback)()=nullptr;
bool (*setPermissionsCallback)(const std::string& path, unsigned permissionsh)=nullptr;

//##################################################################################################
std::vector<std::string> listFiles(const std::string& path, const std::unordered_set<std::string>& extensions)
{
  return listFilesCallback?listFilesCallback(path, extensions):std::vector<std::string>();
}

//##################################################################################################
std::vector<std::string> listFilesOrdered(const std::string& path, const std::unordered_set<std::string>& extensions)
{
  std::vector<std::string> entries = tp_utils::listFiles(path, extensions);
  std::sort(entries.begin(), entries.end());
  return entries;
}

//##################################################################################################
std::vector<std::string> listDirectories(const std::string& path)
{
  return listDirectoriesCallback?listDirectoriesCallback(path):std::vector<std::string>();
}

//##################################################################################################
int64_t fileTimeMS(const std::string& path)
{
  return fileTimeMSCallback?fileTimeMSCallback(path):0;
}

//##################################################################################################
int64_t fileAgeMS(const std::string& path)
{
  return fileAgeMSCallback?fileAgeMSCallback(path):0;
}

//##################################################################################################
bool copyFile(const std::string& pathFrom, const std::string& pathTo)
{
  return copyFileCallback?copyFileCallback(pathFrom, pathTo):false;
}

//##################################################################################################
bool cp(const std::string& pathFrom, const std::string& pathTo, TPRecursive recursive)
{
  return cpCallback?cpCallback(pathFrom, pathTo, recursive):false;
}

//##################################################################################################
bool mv(const std::string& pathFrom, const std::string& pathTo)
{
  return mvCallback?mvCallback(pathFrom, pathTo):false;
}

//##################################################################################################
bool mkdir(const std::string& path, TPCreateFullPath createFullPath)
{
  return mkdirCallback?mkdirCallback(path, createFullPath):false;
}

//##################################################################################################
bool rm(const std::string& path, TPRecursive recursive)
{
  return rmCallback?rmCallback(path, recursive):false;
}

//##################################################################################################
bool exists(const std::string& path)
{
  return existsCallback?existsCallback(path):false;
}

//##################################################################################################
size_t fileSize(const std::string& path)
{
  return fileSizeCallback?fileSizeCallback(path):0;
}

//##################################################################################################
bool setCWD(const std::string& path)
{
  return setCWDCallback?setCWDCallback(path):false;
}

//##################################################################################################
std::string cwd()
{
  return cwdCallback?cwdCallback():std::string();
}

namespace
{
#ifdef TP_WIN32
char del = '\\';
#else
char del = '/';
#endif
}

//##################################################################################################
bool setPermissions(const std::string& path, unsigned permissions)
{
  return setPermissionsCallback?setPermissionsCallback(path, permissions):false;
}

//##################################################################################################
std::string filename(const std::string& path)
{
  std::vector<std::string> results;

  std::string s = path;
  std::replace(s.begin(), s.end(), '\\', '/');

  tpSplit(results, s, '/', TPSplitBehavior::SkipEmptyParts);
  return results.empty()?"":results.back();
}

//##################################################################################################
std::string removeExtension(const std::string& filename)
{
  return filename.substr(0, filename.find_last_of("."));
}

//##################################################################################################
std::string extension(const std::string& filename)
{
  return filename.substr(filename.find_last_of("."));
}

//##################################################################################################
std::string replaceExtension(const std::string& filename, const std::string& ext)
{
  return removeExtension(filename) + ext;
}

//##################################################################################################
std::string directoryName(const std::string& path)
{
  auto i = path.find_last_of("\\/");
  if(i != std::string::npos)
    return path.substr(0, i);
  return "";
}

//##################################################################################################
std::string pathAppend(const std::string& path, const std::string& part)
{
  auto result = path;

  if(del == '\\')
    std::replace(result.begin(), result.end(), '/', '\\');
  else
    std::replace(result.begin(), result.end(), '\\', '/');

  if(!result.empty())
    if(!tpEndsWith(result, std::string(1, del)))
      result.push_back(del);

  return result + part;
}
//##################################################################################################
std::string TP_UTILS_EXPORT appendPaths(const std::vector<std::string>& paths)
{
  std::string result;

  for(const auto& path : paths)
    result = result.empty()?path:pathAppend(result, path);

  return result;
}

//##################################################################################################
std::string pythonJsonPath(std::string path)
{
  std::replace(path.begin(), path.end(), '\\', '/');
  return path;
}


//##################################################################################################
[[nodiscard]]std::string TP_UTILS_EXPORT absolute(const std::string& path)
{
  return absoluteCallback?absoluteCallback(path):std::string();
}

}
