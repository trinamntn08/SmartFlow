#include "PanelWorkspace.h"
#include <QComboBox>
#include <QLabel>
#include <QSignalBlocker>
#include <QSplitter>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <set>
#include <algorithm>

namespace smartflow {
using project::Document;
struct PanelWorkspace::Region {
    QString panel;
    Qt::Orientation orientation = Qt::Horizontal;
    std::unique_ptr<Region> first, second;
    QList<int> sizes{1,1};
    Document metadata = Document::object();
    QWidget* frame = nullptr;
    QSplitter* splitter = nullptr;
};

PanelWorkspace::PanelWorkspace(QWidget* parent) : QWidget(parent), storage(new QWidget(this))
{
    setObjectName("panelWorkspace"); storage->hide();
    root=std::make_unique<Region>();
    auto* layout=new QVBoxLayout(this); layout->setContentsMargins(0,0,0,0);
}

PanelWorkspace::~PanelWorkspace() { changed={}; }

void PanelWorkspace::addPanel(const QString& id, const QString& title, QWidget* widget)
{
    widget->setParent(storage); widget->hide(); panels.push_back({id,title,widget});
}

void PanelWorkspace::rememberSizes(Region& region)
{
    if(!region.first) return;
    if(region.splitter) region.sizes=region.splitter->sizes();
    rememberSizes(*region.first); rememberSizes(*region.second);
}

PanelWorkspace::Region* PanelWorkspace::findPanel(Region& region, const QString& id) const
{
    if(!region.first) return region.panel==id ? &region : nullptr;
    if(auto* found=findPanel(*region.first,id)) return found;
    return findPanel(*region.second,id);
}

QWidget* PanelWorkspace::regionForPanel(const QString& id) const
{
    auto* region=root ? findPanel(*root,id) : nullptr;
    return region ? region->frame : nullptr;
}

void PanelWorkspace::resetLayout()
{
    auto leaf=[](const char* id) { auto region=std::make_unique<Region>(); region->panel=id; return region; };
    auto split=[](Qt::Orientation orientation, std::unique_ptr<Region> a, std::unique_ptr<Region> b, QList<int> sizes) {
        auto region=std::make_unique<Region>(); region->orientation=orientation;
        region->first=std::move(a); region->second=std::move(b); region->sizes=std::move(sizes); return region;
    };
    const bool viewer=std::any_of(panels.begin(),panels.end(),[](const auto& panel) { return panel.id=="viewer"; });
    auto center=viewer ? split(Qt::Vertical,leaf("graph"),leaf("viewer"),{350,380}) : leaf("graph");
    auto right=split(Qt::Vertical,leaf("inspector"),leaf("results"),{350,380});
    root=split(Qt::Horizontal,leaf("library"),split(Qt::Horizontal,std::move(center),std::move(right),{750,320}),{180,1070});
    retained=Document::object(); rebuild(); if(changed) changed();
}

void PanelWorkspace::rebuild()
{
    // Park the actual application widgets before disposing of region chrome.
    // Keeping a QWidget parent avoids disrupting viewer/OpenGL contexts.
    for(const auto& panel : panels) { panel.widget->setParent(storage); panel.widget->hide(); }
    if(body) {
        for(auto* child : body->findChildren<QObject*>()) QObject::disconnect(child,nullptr,this,nullptr);
        layout()->removeWidget(body); body->hide(); body->deleteLater();
    }
    body=buildRegion(*root,this); layout()->addWidget(body); body->show();
}

QWidget* PanelWorkspace::buildRegion(Region& region, QWidget* parent)
{
    auto* frame=new QWidget(parent); region.frame=frame; region.splitter=nullptr;
    auto* layout=new QVBoxLayout(frame); layout->setContentsMargins(0,0,0,0); layout->setSpacing(0);
    if(region.first) {
        auto* splitter=new QSplitter(region.orientation,frame); region.splitter=splitter;
        splitter->setChildrenCollapsible(false);
        splitter->addWidget(buildRegion(*region.first,splitter)); splitter->addWidget(buildRegion(*region.second,splitter));
        splitter->setSizes(region.sizes); layout->addWidget(splitter);
        connect(splitter,&QSplitter::splitterMoved,this,[this] { if(changed) changed(); });
        return frame;
    }
    frame->setMinimumSize(180,100);
    auto* header=new QWidget(frame); auto* controls=new QHBoxLayout(header);
    controls->setContentsMargins(3,3,3,3); controls->setSpacing(2);
    auto* selector=new QComboBox(header); selector->setObjectName("panelChoice");
    selector->setToolTip("Choose the widget for this region. An already visible widget swaps places.");
    selector->addItem("Empty region",QString());
    for(const auto& panel : panels) selector->addItem(panel.title,panel.id);
    auto index=selector->findData(region.panel);
    if(index<0) { selector->addItem("Unavailable widget: "+region.panel,region.panel); index=selector->count()-1; }
    selector->setCurrentIndex(index); controls->addWidget(selector,1);
    auto button=[&](const QString& label, const QString& name, const QString& tooltip, auto callback) {
        auto* tool=new QToolButton(header); tool->setText(label); tool->setObjectName(name);
        tool->setToolTip(tooltip); tool->setAccessibleName(tooltip); tool->setAutoRaise(true);
        controls->addWidget(tool); connect(tool,&QToolButton::clicked,this,callback);
    };
    button("H","splitRegionRight","Split into left and right regions",[this,&region] { splitRegion(region,Qt::Horizontal); });
    button("V","splitRegionBelow","Split into top and bottom regions",[this,&region] { splitRegion(region,Qt::Vertical); });
    button("X","closePanelRegion","Close this region (the widget stays available)",[this,&region] { closeRegion(region); });
    layout->addWidget(header);
    const auto panel=std::find_if(panels.begin(),panels.end(),[&](const auto& item) { return item.id==region.panel; });
    if(panel!=panels.end()) { layout->addWidget(panel->widget,1); panel->widget->show(); }
    else {
        auto* message=new QLabel(region.panel.isEmpty() ? "Choose a widget above, or split this region." : "This widget is unavailable. Its saved region is retained.",frame);
        message->setAlignment(Qt::AlignCenter); message->setWordWrap(true); layout->addWidget(message,1);
    }
    connect(selector,qOverload<int>(&QComboBox::currentIndexChanged),this,[this,&region,selector] { selectPanel(region,selector->currentData().toString()); });
    return frame;
}

void PanelWorkspace::selectPanel(Region& region, const QString& id)
{
    if(region.panel==id) return;
    rememberSizes(*root);
    if(!id.isEmpty()) if(auto* other=findPanel(*root,id)) other->panel=region.panel;
    region.panel=id; rebuild(); if(changed) changed();
}

void PanelWorkspace::splitRegion(Region& region, Qt::Orientation orientation)
{
    int count=0; std::function<void(const Region&)> countLeaves=[&](const Region& item) {
        if(!item.first) ++count; else { countLeaves(*item.first); countLeaves(*item.second); }
    }; countLeaves(*root); if(count>=32) return;
    rememberSizes(*root);
    region.first=std::make_unique<Region>(); region.first->panel=region.panel; region.first->metadata=region.metadata;
    region.second=std::make_unique<Region>(); region.orientation=orientation; region.panel.clear(); region.sizes={1,1};
    region.metadata=Document::object(); rebuild(); if(changed) changed();
}

void PanelWorkspace::closeRegion(Region& region)
{
    rememberSizes(*root);
    if(root.get()==&region) { region.panel.clear(); rebuild(); if(changed) changed(); return; }
    std::function<bool(Region&)> collapse=[&](Region& parent) {
        if(!parent.first) return false;
        if(parent.first.get()==&region || parent.second.get()==&region) {
            auto sibling=parent.first.get()==&region ? std::move(parent.second) : std::move(parent.first);
            parent=std::move(*sibling); return true;
        }
        return collapse(*parent.first) || collapse(*parent.second);
    };
    collapse(*root); rebuild(); if(changed) changed();
}

Document PanelWorkspace::saveLayout() const
{
    std::function<Document(const Region&)> save=[&](const Region& region) {
        auto value=region.metadata;
        if(region.first) {
            value.erase("panel"); value["split"]=region.orientation==Qt::Horizontal ? "horizontal" : "vertical";
            const auto sizes=region.splitter ? region.splitter->sizes() : region.sizes;
            value["sizes"]=Document::array({sizes.value(0,1),sizes.value(1,1)});
            value["children"]=Document::array({save(*region.first),save(*region.second)});
        } else { value.erase("split"); value.erase("sizes"); value.erase("children"); value["panel"]=region.panel.toStdString(); }
        return value;
    };
    auto state=retained; state["version"]=1; state["root"]=save(*root); return state;
}

bool PanelWorkspace::restoreLayout(const Document& state)
{
    if(!state.is_object() || state.value("version",Document())!=1 || !state.contains("root")) return false;
    int count=0; std::set<std::string> assigned;
    std::function<std::unique_ptr<Region>(const Document&,int)> parse=[&](const Document& value,int depth) -> std::unique_ptr<Region> {
        if(!value.is_object() || depth>32 || ++count>63) return {};
        auto region=std::make_unique<Region>(); region->metadata=value;
        if(value.contains("split")) {
            if(value["split"]!="horizontal" && value["split"]!="vertical") return {};
            if(!value.contains("children") || !value["children"].is_array() || value["children"].size()!=2) return {};
            region->orientation=value["split"]=="horizontal" ? Qt::Horizontal : Qt::Vertical;
            region->first=parse(value["children"][0],depth+1); region->second=parse(value["children"][1],depth+1);
            if(!region->first || !region->second) return {};
            if(value.contains("sizes")) {
                const auto& sizes=value["sizes"];
                if(!sizes.is_array() || sizes.size()!=2) return {};
                for(const auto& size : sizes) if(!size.is_number_integer() || size.get<double>()<0 || size.get<double>()>100000) return {};
                if(sizes[0].get<int>()+sizes[1].get<int>()>0) region->sizes={sizes[0].get<int>(),sizes[1].get<int>()};
            }
        } else {
            if(!value.contains("panel") || !value["panel"].is_string()) return {};
            const auto id=value["panel"].get<std::string>();
            if(!id.empty() && !assigned.insert(id).second) return {};
            region->panel=QString::fromStdString(id);
        }
        return region;
    };
    auto candidate=parse(state["root"],0); if(!candidate) return false;
    retained=state; root=std::move(candidate); rebuild(); return true;
}
} // namespace smartflow
