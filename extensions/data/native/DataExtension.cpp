#include "DataExtension.h"
#include "TableViewer.h"
#include <tp_pipeline/StepDelegate.h>
#include <tp_data/AbstractMemberFactory.h>
#include <cmath>
#include <stdexcept>

namespace smartflow::data {
using namespace tp_pipeline;
using tp_utils::StringID;

const StringID& tableType()
{
    static const StringID value("smartflow.data.table@1");
    return value;
}

namespace {
class TableFactory final : public tp_data::AbstractMemberFactory {
public:
    TableFactory() : AbstractMemberFactory(tableType(), "table", {100,180,120}) {}
    std::shared_ptr<tp_data::AbstractMember> clone(std::string& error, const tp_data::AbstractMember& member) const override
    {
        const auto* table=dynamic_cast<const TableMember*>(&member);
        if(!table) { error="Expected table output"; return {}; }
        auto copy=std::make_shared<TableMember>();
        copy->rows=table->rows;
        copy->setName(table->name());
        return copy;
    }
    void save(std::string& error, const tp_data::AbstractMember&, std::string&) const override
    { error="Computed table outputs are not project assets"; }
    std::shared_ptr<tp_data::AbstractMember> load(std::string& error, const std::string&) const override
    { error="Computed table outputs are not project assets"; return {}; }
};

enum class Operation { Sample, Filter, Summary };
class DataDelegate final : public StepDelegate {
public:
    DataDelegate(const char* type, Operation operation)
        : StepDelegate(type,{"Data"}, operation==Operation::Sample ? std::vector<PortDetails>{} :
                       std::vector<PortDetails>{{"in",tableType()}},{{"out",tableType()}}), operation(operation) {}
    void fixupParameters(StepDetails* step, std::vector<StringID>& names) const override
    {
        if(operation==Operation::Summary) return;
        Parameter p;
        p.name=operation==Operation::Sample ? "multiplier" : "minimum";
        p.type=doubleSID(); p.value=operation==Operation::Sample ? 1.0 : 20.0;
        p.min=0.0; p.max=1000.0; p.step=operation==Operation::Sample ? 0.1 : 1.0;
        step->setParamerter(p); names.push_back(p.name);
    }
    bool executeStep(StepContext* context) const override
    {
        auto output=std::make_shared<TableMember>();
        double parameter=0;
        if(operation!=Operation::Summary) {
            const auto p=context->stepDetails->parameter(operation==Operation::Sample ? "multiplier" : "minimum");
            const auto* value=std::get_if<double>(&p.value);
            if(!value || !std::isfinite(*value) || *value<0 || *value>1000)
                throw std::runtime_error("Invalid data parameter");
            parameter=*value;
        }
        if(operation==Operation::Sample) {
            output->rows={{"Alpha",12},{"Beta",25},{"Gamma",7},{"Delta",48},{"Epsilon",31},{"Zeta",19}};
            for(auto& row : output->rows) row.value*=parameter;
        } else {
            const auto* input=context->memberCast<TableMember>("in");
            if(!input) return false;
            if(input->rows.size()>1000) throw std::runtime_error("Table preview supports at most 1000 rows");
            double total=0;
            for(const auto& row : input->rows) {
                if(!std::isfinite(row.value) || std::abs(row.value)>1000000)
                    throw std::runtime_error("Table values exceed supported bounds");
                if(operation==Operation::Filter && row.value>=parameter) output->rows.push_back(row);
                total+=row.value;
            }
            if(operation==Operation::Summary) {
                const auto count=double(input->rows.size());
                output->rows={{"Count",count},{"Total",total},{"Mean",count ? total/count : 0}};
            }
        }
        return context->stepOutput->addSharedMember("out",output,context->progress);
    }
private:
    Operation operation;
};
}

void contribute(WorkspaceConfiguration& configuration)
{
    const struct { const char* name; const char* title; Operation operation; } nodes[] = {
        {"sample","Sample table",Operation::Sample}, {"filter","Filter rows",Operation::Filter},
        {"summary","Summary",Operation::Summary}};
    for(const auto& node : nodes) {
        const auto type=QString("smartflow.data.%1@1").arg(node.name);
        configuration.delegates->addStepDelegate(new DataDelegate(type.toStdString().c_str(),node.operation));
        configuration.nodes.push_back({type,node.title,"Data","smartflow.data",node.name,1});
    }
    configuration.factory->addMemberFactory(new TableFactory);
    configuration.createViewer=[] { return new TableViewer; };
}

WorkspaceConfiguration dataConfiguration()
{
    WorkspaceConfiguration config;
    config.delegates=std::make_shared<StepDelegateMap>();
    config.factory=std::make_shared<tp_data::CollectionFactory>();
    tp_data::createCollectionFactories(*config.factory);
    contribute(config);
    config.factory->finalize();
    config.preset={{"smartflow.data.sample@1",{0,0},{}},
                   {"smartflow.data.filter@1",{240,0},{}},
                   {"smartflow.data.summary@1",{480,0},{}}};
    config.connections={{0,0,1,0},{1,0,2,0}};
    return config;
}
} // namespace smartflow::data
