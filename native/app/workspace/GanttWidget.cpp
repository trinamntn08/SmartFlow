#include "GanttWidget.h"
#include <QTreeWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QScrollBar>
#include <algorithm>
namespace smartflow {
namespace {
class TimelineDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        auto size=QStyledItemDelegate::sizeHint(option,index); size.setHeight(std::max(28,size.height())); return size;
    }
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QStyledItemDelegate::paint(painter,option,index);
        const auto ready=index.data(Qt::UserRole), start=index.data(Qt::UserRole+1), end=index.data(Qt::UserRole+2);
        const double extent=std::max(0.001,index.data(Qt::UserRole+3).toDouble());
        const QRect area=option.rect.adjusted(4,7,-4,-7);
        painter->save(); painter->setClipRect(area);
        auto x=[&](double ms) { return area.left()+int(area.width()*ms/extent); };
        painter->setPen(QColor(120,120,120,70));
        for(int tick=0;tick<=4;++tick) painter->drawLine(x(extent*tick/4),area.top(),x(extent*tick/4),area.bottom());
        auto bar=[&](double from,double to,QColor color) {
            painter->fillRect(QRect(x(from),area.top(),std::max(2,x(to)-x(from)),area.height()),color);
        };
        if(ready.isValid()) bar(ready.toDouble(),start.isValid()?start.toDouble():end.toDouble(),QColor("#d79b35"));
        if(start.isValid()) bar(start.toDouble(),end.toDouble(),QColor("#3694d6"));
        painter->restore();
    }
};
QString stateName(StepState state) {
    switch(state) {
    case StepState::Waiting:return "Waiting"; case StepState::Ready:return "Ready";
    case StepState::Running:return "Running"; case StepState::Succeeded:return "Complete";
    case StepState::Failed:return "Failed"; case StepState::Skipped:return "Skipped";
    case StepState::Cancelled:return "Cancelled";
    } return {};
}
}
GanttWidget::GanttWidget(QWidget* parent):QWidget(parent),tree(new QTreeWidget),summary(new QLabel) {
    setObjectName("ganttWidget"); tree->setObjectName("executionGantt");
    auto* layout=new QVBoxLayout(this); layout->addWidget(summary); layout->addWidget(tree);
    tree->setColumnCount(5); tree->setHeaderLabels({"Node / step","State","Queue ms","Run ms","Timeline (ms)"});
    tree->setRootIsDecorated(false); tree->setUniformRowHeights(true);
    tree->setItemDelegateForColumn(4,new TimelineDelegate(tree));
    tree->header()->setStretchLastSection(true); tree->setColumnWidth(4,300);
    connect(tree,&QTreeWidget::itemClicked,this,[this](QTreeWidgetItem* item) {
        if(selected) selected(item->data(0,Qt::UserRole).toString().toStdString());
    });
}
void GanttWidget::refresh(const GraphProject& project,const std::optional<ExecutionProgressSnapshot>& progress) {
    const int scroll=tree->verticalScrollBar()->value(); tree->clear();
    if(!project.diagnostics().empty()) { summary->setText("Graph cannot execute. Resolve unsupported content or missing inputs before profiling."); return; }
    if(!progress) { summary->setText("Run the graph to inspect process timings."); return; }
    const auto& snapshot=*progress;
    summary->setText(QString("Run %1 | 0 - %2 ms | Amber: ready queue | Blue: execution%3")
        .arg(qulonglong(snapshot.runId)).arg(snapshot.elapsedMs,0,'f',3).arg(snapshot.cancelled?" | Cancelled":""));
    for(const auto* step:project.graph().steps()) {
        const auto found=snapshot.steps.find(step->id()); if(found==snapshot.steps.end()) continue;
        const auto& timing=found->second;
        auto owner=step->id(); QString label=project.title(step->delegateName());
        for(const auto& group:project.resultGroups()) if(std::find(group.steps.begin(),group.steps.end(),step->id())!=group.steps.end()) {
            owner=group.nodeId; label="Component / "+label; break;
        }
        label+=" ["+QString::fromStdString(step->id().toString())+"]";
        const double end=timing.endedMs.value_or(snapshot.elapsedMs);
        const QString queue=timing.readyMs?QString::number(timing.startedMs.value_or(end)-*timing.readyMs,'f',3):"-";
        const QString run=timing.startedMs?QString::number(end-*timing.startedMs,'f',3):"-";
        auto* item=new QTreeWidgetItem(tree,{label,stateName(timing.state),queue,run,{}});
        item->setData(0,Qt::UserRole,QString::fromStdString(owner.toString()));
        if(timing.readyMs) item->setData(4,Qt::UserRole,*timing.readyMs);
        if(timing.startedMs) item->setData(4,Qt::UserRole+1,*timing.startedMs);
        // Queued but never invoked tasks stop at the frozen run endpoint.
        item->setData(4,Qt::UserRole+2,end); item->setData(4,Qt::UserRole+3,snapshot.elapsedMs);
        item->setToolTip(4,QString("Queue: %1 ms; execution: %2 ms\n%3").arg(queue,run,QString::fromStdString(timing.error)));
    }
    for(int column=0;column<4;++column) tree->resizeColumnToContents(column);
    tree->verticalScrollBar()->setValue(scroll);
}
}
