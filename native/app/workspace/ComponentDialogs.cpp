#include "ComponentDialogs.h"
#include "WorkspaceWindow.h"
#include <QMenuBar>
#include "project/ComponentFile.h"
#include <tp_pipeline/StepDelegate.h>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <QMessageBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QTabWidget>
#include <QUuid>
#include <QVBoxLayout>
#include <algorithm>
#include <set>

namespace smartflow {
namespace {
using project::Document;
QString text(const std::string& value) { return QString::fromStdString(value); }
QLabel* message(QWidget* parent, const QString& name)
{
    auto* label=new QLabel(parent);
    label->setObjectName(name);
    label->setWordWrap(true);
    label->setTextFormat(Qt::PlainText);
    return label;
}
QLineEdit* field(QFormLayout* form, const QString& label, const QString& name, const QString& value)
{
    auto* edit=new QLineEdit(value);
    edit->setObjectName(name);
    form->addRow(label,edit);
    return edit;
}
QString freshId() { return QUuid::createUuid().toString(QUuid::WithoutBraces); }
bool endpointMatches(const Document& a, const Document& b)
{
    return a["nodeId"]==b["nodeId"] && a["portId"]==b["portId"];
}
QString nodeLabel(GraphProject& project, const std::string& id)
{
    const auto* step=project.step(id);
    const auto title=step ? project.title(step->delegateName()) : "Unavailable";
    int index=0;
    for(const auto& node : project.selectedGraph()["nodes"]) {
        ++index;
        if(node["id"]==id) return title+" ("+QString::number(index)+")";
    }
    return title;
}
}

ComponentAuthorDialog::ComponentAuthorDialog(GraphProject& project, std::vector<std::string> selection, QWidget* parent,
    Document seed, bool publish)
    : QDialog(parent), graphProject(project), selection(std::move(selection)), revision(project.revision()),
      seed(std::move(seed)), publish(publish)
{
    setObjectName("componentAuthor");
    setWindowTitle("Create component from selection");
    resize(720,560);
    auto* layout=new QVBoxLayout(this);
    auto* hint=message(this,"componentAuthorHint");
    hint->setText("Save the selected nodes as a reusable component. Boundary inputs and outputs are required. Choose additional outputs and controls to expose. The original nodes remain on the canvas.");
    layout->addWidget(hint);
    auto* form=new QFormLayout;
    title=field(form,"Title","componentTitle","New component");
    if(this->seed.is_object()) {
        setWindowTitle("Save edited component copy");
        title->setText(text(this->seed["title"])+" copy");
        hint->setText("Choose the edited copy's title and exposed interface. Saving adds a new definition; existing definitions and instances retain their snapshots.");
    }
    identity=freshId().toStdString();
    layout->addLayout(form);
    auto* tabs=new QTabWidget;
    layout->addWidget(tabs,1);
    const std::set<std::string> selected(this->selection.begin(),this->selection.end());
    std::map<std::string,QTableWidget*> tables;
    for(const auto& section : {"inputs","outputs","controls"}) {
        auto* table=new QTableWidget(0,3);
        table->setObjectName("component_"+QString(section));
        table->setHorizontalHeaderLabels({"Expose","Node / endpoint","Name"});
        table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Stretch);
        table->verticalHeader()->hide();
        tables[section]=table;
        auto label=QString(section); label[0]=label[0].toUpper();
        tabs->addTab(table,label);
    }
    auto add=[&](const std::string& section, Document endpoint, const QString& label, bool required, bool checked) {
        auto* table=tables.at(section);
        const auto row=table->rowCount(); table->insertRow(row);
        auto* expose=new QCheckBox(required ? "Required" : "Expose");
        expose->setChecked(required || checked); expose->setEnabled(!required);
        expose->setObjectName(text(section)+"Expose"+QString::number(row));
        auto* name=new QLineEdit(text(section.substr(0,section.size()-1))+QString::number(row+1));
        if(this->seed.is_object()) {
            expose->setChecked(required);
            for(const auto& entry : this->seed[section]) {
                const auto& saved=entry[section=="outputs" ? "source" : "target"];
                if(saved["nodeId"]==endpoint["nodeId"] &&
                   saved[section=="controls" ? "parameter" : "portId"]==endpoint[section=="controls" ? "parameter" : "portId"]) {
                    expose->setChecked(true); name->setText(text(entry["id"])); break;
                }
            }
        }
        name->setObjectName(text(section)+"Name"+QString::number(row));
        name->setEnabled(expose->isChecked());
        connect(expose,&QCheckBox::toggled,name,&QWidget::setEnabled);
        auto* item=new QTableWidgetItem(label); item->setFlags(Qt::ItemIsEnabled);
        table->setCellWidget(row,0,expose); table->setItem(row,1,item); table->setCellWidget(row,2,name);
        rows.push_back({section,std::move(endpoint),expose,name});
    };
    QStringList unavailable;
    const auto& graph=project.selectedGraph();
    for(const auto& node : graph["nodes"]) {
        const auto id=node["id"].get<std::string>();
        if(!selected.count(id)) continue;
        auto* step=project.step(id);
        if(!step || project::GraphComponent::isInstance(node)) { unavailable << nodeLabel(project,id); continue; }
        const auto* delegate=project.registry()->stepDelegate(step->delegateName());
        for(const auto& port : delegate->inPorts()) {
            const Document endpoint={{"nodeId",id},{"portId",port.name.toString()}};
            bool internal=false, boundary=false;
            for(const auto& edge : graph["connections"]) if(endpointMatches(edge["target"],endpoint)) {
                internal=selected.count(edge["source"]["nodeId"].get<std::string>());
                boundary=!internal;
            }
            if(!internal) add("inputs",endpoint,nodeLabel(project,id)+" / "+text(port.name.toString()),boundary,true);
        }
        for(const auto& port : delegate->outPorts()) {
            const Document endpoint={{"nodeId",id},{"portId",port.name.toString()}};
            bool consumer=false, boundary=false;
            for(const auto& edge : graph["connections"]) if(endpointMatches(edge["source"],endpoint)) {
                consumer=true;
                if(!selected.count(edge["target"]["nodeId"].get<std::string>())) boundary=true;
            }
            add("outputs",endpoint,nodeLabel(project,id)+" / "+text(port.name.toString()),boundary,!consumer);
        }
        for(const auto& [name,parameter] : step->parameters())
            if(node["parameters"].contains(name.toString()))
                add("controls",{{"nodeId",id},{"parameter",name.toString()}},nodeLabel(project,id)+" / "+text(name.toString()),false,false);
    }
    error=message(this,"componentError"); layout->addWidget(error);
    auto* buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Save)->setText("Create component");
    buttons->button(QDialogButtonBox::Save)->setObjectName("createComponentConfirm");
    layout->addWidget(buttons);
    connect(buttons,&QDialogButtonBox::accepted,this,&ComponentAuthorDialog::accept);
    connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
    if(selected.empty() || !unavailable.empty()) {
        error->setText(selected.empty() ? "Select at least one node." : "Select available ordinary nodes. Nested components are not supported.");
        buttons->button(QDialogButtonBox::Save)->setEnabled(false);
    }
}

void ComponentAuthorDialog::accept()
{
    try {
        if(revision!=graphProject.revision()) throw project::FileError("The graph changed. Reopen this dialog to use the current selection.");
        auto inputs=Document::array(), outputs=Document::array(), controls=Document::array();
        for(const auto& row : rows) if(row.expose->isChecked()) {
            auto& entries=row.section=="inputs" ? inputs : row.section=="outputs" ? outputs : controls;
            entries.push_back({{"id",row.name->text().trimmed().toStdString()},
                {row.section=="outputs" ? "source" : "target",row.endpoint}});
        }
        auto extracted=project::GraphComponent::extract(graphProject.retained(),
            graphProject.selectedGraph()["id"],selection,identity,
            title->text().trimmed().toStdString(),inputs,outputs,controls).definition();
        if(seed.is_object()) {
            authored=seed;
            for(const auto* key : {"id","title","graph","inputs","outputs","controls"}) authored[key]=extracted[key];
            for(const auto* section : {"inputs","outputs","controls"}) for(auto& entry : authored[section])
                for(const auto& saved : seed[section]) {
                    const auto* key=std::string(section)=="outputs" ? "source" : "target";
                    const auto* port=std::string(section)=="controls" ? "parameter" : "portId";
                    if(saved[key]["nodeId"]==entry[key]["nodeId"] && saved[key][port]==entry[key][port]) {
                        auto retained=saved; retained["id"]=entry["id"]; entry=std::move(retained); break;
                    }
                }
        } else authored=std::move(extracted);
        const project::GraphComponent component(authored);
        if(publish) graphProject.commands().catalogComponent(component);
        QDialog::accept();
    } catch(const std::exception& exception) { error->setText(QString::fromUtf8(exception.what())); }
}

ComponentEditDialog::ComponentEditDialog(GraphProject& destination, WorkspaceConfiguration configuration,
    project::GraphComponent component, QWidget* parent)
    : QDialog(parent), destination(destination), revision(destination.revision()), original(component.definition())
{
    setObjectName("componentEditor"); setWindowTitle("Edit a component copy"); resize(1200,850);
    auto envelope=project::create("component-draft");
    envelope["project"]["graphs"]=Document::array({original["graph"]});
    project::DocumentSession inspection(envelope,original["graph"]["id"],configuration.delegates,configuration.nodes);
    inspection.validateComponent(component);
    configuration.preset.clear(); configuration.connections.clear();
    auto* layout=new QVBoxLayout(this);
    auto* hint=message(this,"componentEditorHint");
    hint->setText("Edit the body using the canvas and inspector. Save copy chooses its title and exposed interface. Cancel discards the draft. Exposed inputs are unbound here; test their execution after inserting the copy in a project.");
    layout->addWidget(hint);
    draft=new WorkspaceWindow(std::move(configuration));
    draft->setParent(this,Qt::Widget); draft->menuBar()->hide();
    draft->execution().setLive(false);
    draft->findChild<QCheckBox*>("liveUpdates")->setChecked(false);
    draft->loadDocument(std::move(envelope)); layout->addWidget(draft,1);
    error=message(this,"componentEditorError"); layout->addWidget(error);
    auto* buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Save)->setText("Save copy...");
    buttons->button(QDialogButtonBox::Save)->setObjectName("saveComponentCopy"); layout->addWidget(buttons);
    connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
    connect(buttons,&QDialogButtonBox::accepted,this,[this] {
        try {
            if(revision!=this->destination.revision()) throw project::FileError("The destination graph changed. Reopen the component editor.");
            std::vector<std::string> nodes;
            for(const auto& node : draft->project().selectedGraph()["nodes"]) nodes.push_back(node["id"]);
            ComponentAuthorDialog author(draft->project(),std::move(nodes),this,original,false);
            if(author.exec()==QDialog::Accepted) saveCopy(project::GraphComponent(author.definition()));
        } catch(const std::exception& exception) { error->setText(QString::fromUtf8(exception.what())); }
    });
}

void ComponentEditDialog::saveCopy(const project::GraphComponent& component)
{
    if(revision!=destination.revision()) throw project::FileError("The destination graph changed. Reopen the component editor.");
    if(component.definition()["id"]==original["id"]) throw project::FileError("An edited copy requires a fresh identity.");
    destination.commands().session().validateComponent(component);
    destination.commands().catalogComponent(component);
    QDialog::accept();
}

ComponentLibraryDialog::ComponentLibraryDialog(GraphProject& project, QWidget* parent)
    : QDialog(parent), graphProject(project), revision(project.revision()),
      catalog(project.retained()["project"].value("components",Document::array()))
{
    setObjectName("componentLibrary"); setWindowTitle("Component library"); resize(660,500);
    auto* layout=new QVBoxLayout(this);
    auto* hint=message(this,"componentLibraryHint");
    hint->setText("Choose a saved component, connect its inputs and adjust its controls. Insert it as one node, or expand its ordinary nodes. Select the inserted node to inspect or pin its output.");
    layout->addWidget(hint);
    auto* form=new QFormLayout;
    library=new QComboBox; library->setObjectName("componentCatalog"); form->addRow("Component",library);
    identity=freshId().toStdString(); layout->addLayout(form);
    collapsed=new QCheckBox("Insert as one node");
    collapsed->setObjectName("componentCollapsed"); collapsed->setChecked(true); layout->addWidget(collapsed);
    auto* files=new QHBoxLayout;
    auto* importButton=new QPushButton("Import component..."); importButton->setObjectName("importComponentFile");
    exportButton=new QPushButton("Export component..."); exportButton->setObjectName("exportComponentFile");
    removeButton=new QPushButton("Remove from library..."); removeButton->setObjectName("removeCatalogComponent");
    editButton=new QPushButton("Edit a copy..."); editButton->setObjectName("editComponentCopy");
    files->addWidget(importButton); files->addWidget(exportButton); files->addWidget(editButton); files->addWidget(removeButton); files->addStretch(); layout->addLayout(files);
    auto* scroll=new QScrollArea; scroll->setWidgetResizable(true);
    auto* body=new QWidget; bindings=new QFormLayout(body); scroll->setWidget(body); layout->addWidget(scroll,1);
    error=message(this,"componentError"); layout->addWidget(error);
    auto* buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    insert=buttons->button(QDialogButtonBox::Ok); insert->setText("Insert component"); insert->setObjectName("insertComponentConfirm");
    layout->addWidget(buttons);
    connect(buttons,&QDialogButtonBox::accepted,this,&ComponentLibraryDialog::accept);
    connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
    connect(library,qOverload<int>(&QComboBox::currentIndexChanged),this,[this] { refreshBindings(); });
    connect(importButton,&QPushButton::clicked,this,[this] {
        const auto path=QFileDialog::getOpenFileName(this,"Import component",{},"SmartFlow components (*.smartflow-component *.json);;All files (*)");
        if(path.isEmpty()) return;
        try { importFile(path); }
        catch(const std::exception& exception) { error->setText(QString::fromUtf8(exception.what())); }
    });
    connect(exportButton,&QPushButton::clicked,this,[this] {
        const auto path=QFileDialog::getSaveFileName(this,"Export component","component.smartflow-component","SmartFlow components (*.smartflow-component);;JSON (*.json)");
        if(path.isEmpty()) return;
        try { exportFile(path); }
        catch(const std::exception& exception) { error->setText(QString::fromUtf8(exception.what())); }
    });
    connect(removeButton,&QPushButton::clicked,this,[this] {
        try {
            if(revision!=graphProject.revision()) throw project::FileError("The graph changed. Reopen this dialog before removing a component.");
            QMessageBox confirm(QMessageBox::Question,"Remove component from library",{},QMessageBox::Yes|QMessageBox::No,this);
            confirm.setObjectName("confirmRemoveComponent"); confirm.setTextFormat(Qt::PlainText);
            confirm.setText("Remove \""+library->currentText()+"\" from this project's library?\n\nExisting instances keep their saved snapshots and remain usable. Undo restores the library entry.");
            confirm.setDefaultButton(QMessageBox::No);
            if(confirm.exec()!=QMessageBox::Yes) return;
            removeSelected();
        } catch(const std::exception& exception) { error->setText(QString::fromUtf8(exception.what())); }
    });
    connect(editButton,&QPushButton::clicked,this,[this] {
        try {
            if(revision!=graphProject.revision()) throw project::FileError("The graph changed. Reopen the library before editing.");
            auto* window=qobject_cast<WorkspaceWindow*>(parentWidget());
            if(!window) throw project::FileError("Open the component library from the workspace to edit a copy.");
            ComponentEditDialog editor(graphProject,window->configuration(),project::GraphComponent(catalog.at(size_t(library->currentIndex()))),this);
            if(editor.exec()==QDialog::Accepted) {
                revision=graphProject.revision();
                refreshCatalog(int(graphProject.retained()["project"]["components"].size())-1);
            }
        } catch(const std::exception& exception) { error->setText(QString::fromUtf8(exception.what())); }
    });
    refreshCatalog();
}

void ComponentLibraryDialog::refreshCatalog(int selected)
{
    const QSignalBlocker blocker(library);
    catalog=graphProject.retained()["project"].value("components",Document::array());
    library->clear();
    if(catalog.is_array()) for(const auto& definition : catalog) {
        QString label="Unavailable component";
        if(definition.is_object() && definition.contains("title") && definition["title"].is_string()) label=text(definition["title"]);
        library->addItem(label);
    }
    library->setCurrentIndex(selected);
    refreshBindings();
}

void ComponentLibraryDialog::importFile(const QString& path)
{
    if(revision!=graphProject.revision()) throw project::FileError("The graph changed. Reopen this dialog before importing.");
    const auto component=project::readComponent(path);
    graphProject.commands().catalogComponent(component);
    revision=graphProject.revision();
    const auto& current=graphProject.retained()["project"]["components"];
    int index=0;
    for(const auto& entry : current) {
        if(entry==component.definition()) break;
        ++index;
    }
    refreshCatalog(index);
}

void ComponentLibraryDialog::exportFile(const QString& path) const
{
    if(revision!=graphProject.revision()) throw project::FileError("The graph changed. Reopen this dialog before exporting.");
    if(!catalog.is_array() || library->currentIndex()<0) throw project::FileError("Choose a saved component to export.");
    project::writeComponent(path,project::GraphComponent(catalog[size_t(library->currentIndex())]));
}

void ComponentLibraryDialog::removeSelected()
{
    if(revision!=graphProject.revision()) throw project::FileError("The graph changed. Reopen this dialog before removing a component.");
    const auto selected=library->currentIndex();
    if(!catalog.is_array() || selected<0) throw project::FileError("Choose a saved component to remove.");
    graphProject.commands().removeCatalogComponent(size_t(selected));
    revision=graphProject.revision();
    const auto remaining=graphProject.retained()["project"]["components"].size();
    refreshCatalog(remaining ? std::min(selected,int(remaining)-1) : -1);
}

void ComponentLibraryDialog::refreshBindings()
{
    while(bindings->rowCount()) bindings->removeRow(0);
    inputs.clear(); controls.clear(); error->clear(); insert->setEnabled(false);
    exportButton->setEnabled(false);
    editButton->setEnabled(false);
    removeButton->setEnabled(catalog.is_array() && library->currentIndex()>=0);
    try {
        if(!catalog.is_array()) throw project::FileError("The saved component library has an unsupported format. Its content is retained.");
        if(library->currentIndex()<0) throw project::FileError("No saved components. Import a component, or select nodes and choose Create from selection.");
        const project::GraphComponent component(catalog[size_t(library->currentIndex())]);
        exportButton->setEnabled(true); // Structural export does not need installed packages.
        auto envelope=project::create("component-inspection");
        envelope["project"]["graphs"]=Document::array({component.definition()["graph"]});
        project::DocumentSession body(envelope,component.definition()["graph"]["id"],graphProject.registry(),graphProject.nodePresentations());
        for(const auto& node : component.definition()["graph"]["nodes"])
            if(!body.inspectNode(node["id"])) throw project::FileError("This component requires an unavailable node package or version. Its definition is retained.");
        editButton->setEnabled(qobject_cast<WorkspaceWindow*>(parentWidget())!=nullptr);
        bool complete=true;
        for(const auto& input : component.definition()["inputs"]) {
            const auto id=input["id"].get<std::string>();
            const auto target=body.inspectNode(input["target"]["nodeId"]);
            const auto* delegate=graphProject.registry()->stepDelegate(target->delegateName());
            const auto& ports=delegate->inPorts();
            const auto port=std::find_if(ports.begin(),ports.end(),[&](const auto& item) { return item.name.toString()==input["target"]["portId"]; });
            if(port==ports.end()) throw project::FileError("Component input port is unavailable.");
            auto* combo=new QComboBox; combo->setObjectName("componentInput_"+text(id));
            combo->addItem("Choose an output...");
            for(const auto& node : graphProject.selectedGraph()["nodes"]) {
                const auto nodeId=node["id"].get<std::string>();
                const auto* source=graphProject.step(nodeId); if(!source) continue;
                for(const auto& output : source->outputMapping())
                    if(output.portType==port->type) {
                        Document endpoint={{"nodeId",nodeId},{"portId",output.portName.toString()}};
                        combo->addItem(nodeLabel(graphProject,nodeId)+" / "+text(output.portName.toString()),text(endpoint.dump()));
                    }
            }
            if(combo->count()==1) complete=false;
            bindings->addRow("Input: "+text(id),combo); inputs.emplace_back(id,combo);
        }
        for(const auto& control : component.definition()["controls"]) {
            const auto id=control["id"].get<std::string>();
            const auto node=body.inspectNode(control["target"]["nodeId"]);
            const auto parameter=node->parameter(control["target"]["parameter"].get<std::string>());
            if(parameter.type!=tp_pipeline::doubleSID()) {
                bindings->addRow("Control: "+text(id),new QLabel("Uses the saved default; editor unavailable."));
                continue;
            }
            auto* spin=new QDoubleSpinBox; spin->setObjectName("componentControl_"+text(id));
            spin->setDecimals(10);
            spin->setRange(tpGetVariantValue<double>(parameter.min,-1000000),tpGetVariantValue<double>(parameter.max,1000000));
            spin->setSingleStep(tpGetVariantValue<double>(parameter.step,1));
            spin->setValue(tpGetVariantValue<double>(parameter.value));
            bindings->addRow("Control: "+text(id),spin); controls.emplace_back(id,spin,spin->value());
        }
        QStringList outputs;
        for(const auto& output : component.definition()["outputs"]) outputs << text(output["id"]);
        auto* outputLabel=message(this,"componentOutputs");
        outputLabel->setText(outputs.empty() ? "None" : outputs.join(", "));
        bindings->addRow("Outputs",outputLabel);
        insert->setEnabled(complete);
        if(!complete) error->setText("No compatible output exists for one or more inputs. Add a source node before inserting this component.");
    } catch(const std::exception& exception) { error->setText(QString::fromUtf8(exception.what())); }
}

void ComponentLibraryDialog::accept()
{
    try {
        if(!insert->isEnabled()) return;
        if(revision!=graphProject.revision()) throw project::FileError("The graph changed. Reopen the component library to use the current graph.");
        auto sources=Document::object(), values=Document::object();
        for(const auto& [id,combo] : inputs) {
            if(combo->currentIndex()==0) throw project::FileError("Choose an output for input: "+id);
            sources[id]=Document::parse(combo->currentData().toString().toStdString());
        }
        for(const auto& [id,spin,initial] : controls)
            if(spin->value()!=initial) values[id]=spin->value();
        graphProject.commands().instantiateComponent(project::GraphComponent(catalog[size_t(library->currentIndex())]),
            identity,values,sources,collapsed->isChecked());
        QDialog::accept();
    } catch(const std::exception& exception) { error->setText(QString::fromUtf8(exception.what())); }
}
} // namespace smartflow
