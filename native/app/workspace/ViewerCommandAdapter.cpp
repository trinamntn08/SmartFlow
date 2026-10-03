#include "ViewerCommandAdapter.h"
#include <QPointer>
#include <cmath>
#include <limits>
#include <set>

namespace smartflow {
namespace {
struct Target { std::string node, parameter; ViewerParameter value; };
std::optional<Target> resolve(GraphProject& project,const QString& executionNode,const QString& parameter,
                              const std::function<QString()>& scope)
{
    const auto allowed=scope ? scope() : QString();
    if(scope && allowed.isEmpty()) return {};
    for(const auto& node : project.selectedGraph()["nodes"]) {
        const auto id=node["id"].get<std::string>(); std::string field;
        if(scope && QString::fromStdString(id)!=allowed) continue;
        if(project::GraphComponent::isInstance(node)) {
            try {
                if(node["version"]!=1) continue;
                const auto expanded=project::GraphComponent(node["component"]).expandBody(id,node["parameters"]);
                for(auto control=expanded.bindings.controls.begin();control!=expanded.bindings.controls.end();++control)
                    if(control.value()["nodeId"]==executionNode.toStdString() &&
                       control.value()["parameter"]==parameter.toStdString()) { field=control.key(); break; }
            } catch(const std::exception&) { continue; }
        } else if(!scope && id==executionNode.toStdString()) field=parameter.toStdString();
        if(field.empty()) continue;
        auto* step=project.step(id); if(!step) return {};
        const auto p=step->parameter(field);
        const auto* value=std::get_if<double>(&p.value);
        const auto* minimum=std::get_if<double>(&p.min); const auto* maximum=std::get_if<double>(&p.max);
        if(p.type!=tp_pipeline::doubleSID() || !p.enabled || !value || !std::isfinite(*value)) return {};
        const double low=minimum ? *minimum : -std::numeric_limits<double>::max();
        const double high=maximum ? *maximum : std::numeric_limits<double>::max();
        if(!std::isfinite(low) || !std::isfinite(high) || low>high) return {};
        return Target{id,field,{*value,low,high}};
    }
    return {};
}
}
void attachViewerCommands(OutputViewer& viewer,GraphProject& project,std::function<QString()> scope)
{
    const QPointer<GraphProject> document(&project);
    viewer.commands.revision=[document] { return document ? document->revision() : quint64(0); };
    viewer.commands.parameter=[document,scope](const QString& node,const QString& parameter)->std::optional<ViewerParameter> {
        if(!document) return {};
        const auto target=resolve(*document,node,parameter,scope);
        return target ? std::optional<ViewerParameter>(target->value) : std::nullopt;
    };
    viewer.commands.apply=[document,scope](const ViewerEditRequest& request)->QString {
        if(!document) return "Project is no longer available.";
        if(request.revision!=document->revision()) return "Project changed during the gesture. Try again.";
        if(request.parameters.empty() || request.parameters.size()>64) return "Invalid viewer command.";
        std::vector<std::pair<Target,double>> changes; std::set<std::pair<std::string,std::string>> unique;
        for(const auto& edit : request.parameters) {
            const auto target=resolve(*document,edit.executionNode,edit.parameter,scope);
            if(!target) return "This parameter is unavailable or not exposed by the component.";
            if(!std::isfinite(edit.value) || edit.value<target->value.minimum || edit.value>target->value.maximum)
                return "Viewer parameter is outside its supported range.";
            if(!unique.emplace(target->node,target->parameter).second) return "Duplicate viewer parameter.";
            changes.emplace_back(*target,edit.value);
        }
        try {
            document->commands().edit(request.label.isEmpty() ? "Viewer edit" : request.label,[&](project::DocumentSession& candidate) {
                for(const auto& [target,value] : changes) candidate.setParameter(target.node,target.parameter,value);
            });
            return {};
        } catch(const std::exception& error) { return QString::fromUtf8(error.what()); }
    };
    QObject::connect(&project,&GraphProject::changed,&viewer,[&viewer] { viewer.projectChanged(); });
}
}
