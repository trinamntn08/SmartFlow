#pragma once
#include <nlohmann/json.hpp>
#include <QByteArray>
#include <QString>
#include <stdexcept>

namespace smartflow::project {
using Document = nlohmann::json;

class FileError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

constexpr qsizetype MaximumFileBytes = 16 * 1024 * 1024;
constexpr int MaximumNesting = 128;

// Shared strict JSON transport for project and component files. Domain/schema
// validation belongs to the caller; duplicate keys and lossy numbers reject.
namespace jsonFile {
Document parse(const QByteArray& bytes);
QByteArray serialize(const Document& document);
Document read(const QString& path);
void write(const QString& path, const Document& document);
}

// Schema v1 is shared with packages/core. These functions do not consult an
// extension registry or repair unresolved graph connections.
Document create(const std::string& projectId);
void validate(const Document& document);
Document parse(const QByteArray& bytes);
QByteArray serialize(const Document& document);
Document read(const QString& path);
// Validate before opening a temporary file, then atomically replace the target.
// No direct-write fallback: failed writes must not truncate an existing project.
void write(const QString& path, const Document& document);
} // namespace smartflow::project
