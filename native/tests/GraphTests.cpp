#include "NumberNode.h"
#include <QtNodes/DataFlowGraphModel>
#include <QtTest/QtTest>

class GraphTests : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void rejectCyclesAndOccupiedInput()
    {
        auto registry = std::make_shared<QtNodes::NodeDelegateModelRegistry>();
        registry->registerModel<NumberNode>();
        QtNodes::DataFlowGraphModel graph(registry);
        const auto first = graph.addNode("Number");
        const auto second = graph.addNode("Number");
        const auto third = graph.addNode("Number");
        graph.addConnection({first, 0, second, 0});
        QVERIFY(!graph.connectionPossible({second, 0, first, 0}));
        QVERIFY(!graph.connectionPossible({third, 0, second, 0}));
        QVERIFY(!graph.connectionPossible({first, 0, first, 0}));
    }
    void connectAndPropagate()
    {
        auto registry = std::make_shared<QtNodes::NodeDelegateModelRegistry>();
        registry->registerModel<NumberNode>();
        QtNodes::DataFlowGraphModel graph(registry);
        const auto source = graph.addNode("Number");
        const auto target = graph.addNode("Number");
        const QtNodes::ConnectionId link{source, 0, target, 0};
        QVERIFY(graph.connectionPossible(link));
        graph.addConnection(link);
        graph.delegateModel<NumberNode>(source)->load({{"value", 42}});
        const auto output = std::dynamic_pointer_cast<NumberData>(graph.delegateModel<NumberNode>(target)->outData(0));
        QVERIFY(output);
        QCOMPARE(output->value, 42.0);
        QVERIFY(graph.deleteConnection(link));
        QCOMPARE(std::dynamic_pointer_cast<NumberData>(graph.delegateModel<NumberNode>(target)->outData(0))->value, 1.0);
        QVERIFY(graph.deleteNode(source));
        QCOMPARE(graph.allNodeIds().size(), size_t(1));
    }
    void canvasSnapshotRoundTrip()
    {
        auto registry = std::make_shared<QtNodes::NodeDelegateModelRegistry>();
        registry->registerModel<NumberNode>();
        QtNodes::DataFlowGraphModel graph(registry);
        const auto node = graph.addNode("Number");
        graph.delegateModel<NumberNode>(node)->load({{"value", 17}});
        graph.setNodeData(node, QtNodes::NodeRole::Position, QPointF(12, 34));
        const auto snapshot = graph.save();
        QtNodes::DataFlowGraphModel restored(registry);
        restored.load(snapshot);
        QCOMPARE(restored.save(), snapshot);
    }
};
QTEST_MAIN(GraphTests)
#include "GraphTests.moc"
