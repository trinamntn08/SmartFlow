#pragma once
#include "project/ProjectFile.h"
#include <QWidget>
#include <functional>
#include <memory>
#include <vector>

namespace smartflow {
// Selectable split regions. Widgets remain alive while moved or hidden;
// layout state is independent of graph semantics and extension domains.
class PanelWorkspace final : public QWidget {
    Q_OBJECT
public:
    explicit PanelWorkspace(QWidget* parent = nullptr);
    ~PanelWorkspace() override;
    void addPanel(const QString& id, const QString& title, QWidget* widget);
    void resetLayout();
    project::Document saveLayout() const;
    bool restoreLayout(const project::Document& state);
    QWidget* regionForPanel(const QString& id) const;
    std::function<void()> changed;
private:
    struct Region;
    struct Panel { QString id, title; QWidget* widget; };
    void rebuild();
    QWidget* buildRegion(Region& region, QWidget* parent);
    void selectPanel(Region& region, const QString& id);
    void splitRegion(Region& region, Qt::Orientation orientation);
    void closeRegion(Region& region);
    void rememberSizes(Region& region);
    Region* findPanel(Region& region, const QString& id) const;
    std::unique_ptr<Region> root;
    std::vector<Panel> panels;
    QWidget* storage;
    QWidget* body = nullptr;
    project::Document retained = project::Document::object();
};
} // namespace smartflow
