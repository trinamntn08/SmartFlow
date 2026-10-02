#pragma once

#include <tp_data/Collection.h>
#include <tp_data/CollectionFactory.h>
#include <tp_data/AbstractMember.h>
#include <stdexcept>

namespace smartflow {
// Legacy inputs expose mutable pointers. Clone at execution boundaries so each
// invocation owns its members, including recursively owned domain payloads.
// Factories must implement deep clones; retaining a source alias is unsupported.
inline std::shared_ptr<tp_data::Collection> cloneExecutionData(
    const tp_data::Collection& source, const tp_data::CollectionFactory& factory)
{
    auto output = std::make_shared<tp_data::Collection>();
    for(const auto& member : source.members()) {
        if(!member) throw std::runtime_error("Null execution member");
        std::string error;
        auto copy = factory.clone(error, *member);
        if(!error.empty() || !copy || copy.get() == member.get() || copy->type() != member->type())
            throw std::runtime_error("Cannot isolate execution data: " +
                (error.empty() ? member->type().toString() : error));
        copy->setName(member->name());
        copy->setTimestampMS(member->timestampMS());
        output->addMember(copy);
    }
    return output;
}
} // namespace smartflow
