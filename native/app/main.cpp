#include "NumberNode.h"
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/GraphicsView>
#include <QApplication>
#include <QMainWindow>
#include <QStatusBar>
#include <QTimer>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("SmartFlow");
    auto registry = std::make_shared<QtNodes::NodeDelegateModelRegistry>();
    registry->registerModel<NumberNode>("Data");
    QtNodes::DataFlowGraphModel graph(registry);
    QtNodes::DataFlowGraphicsScene scene(graph);
    QMainWindow window;
    window.setWindowTitle("SmartFlow — Native migration preview");
    window.setCentralWidget(new QtNodes::GraphicsView(&scene));
    window.statusBar()->showMessage("Right-click to add a Number node. Drag ports to connect. 3D migration is pending.");
    const auto first = graph.addNode("Number");
    const auto second = graph.addNode("Number");
    graph.setNodeData(first, QtNodes::NodeRole::Position, QPointF(0, 0));
    graph.setNodeData(second, QtNodes::NodeRole::Position, QPointF(320, 0));
    graph.addConnection({first, 0, second, 0});
    window.resize(1100, 720);
    window.show();
    if (app.arguments().contains("--smoke-test")) {
        QTimer::singleShot(250, &app, [&] {
            const auto index = app.arguments().indexOf("--screenshot");
            if (index >= 0 && (index + 1 >= app.arguments().size() ||
                !window.grab().save(app.arguments().at(index + 1)))) {
                app.exit(2);
                return;
            }
            app.quit();
        });
    }
    return app.exec();
}
