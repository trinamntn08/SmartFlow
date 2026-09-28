#pragma once

#include <QtGui/QColor>

#include "Export.hpp"
#include "Style.hpp"

namespace QtNodes {

class NODE_EDITOR_PUBLIC ConnectionStyle : public Style
{
public:
    ConnectionStyle();

    ConnectionStyle(QString jsonText);

    ~ConnectionStyle() = default;

public:
    static void setConnectionStyle(QString jsonText);

public:
    void loadJson(QJsonObject const &json) override;

    QJsonObject toJson() const override;

    void addNormalColor(const QString& typeId, const QColor& color);

public:
    QColor constructionColor() const;
    QColor normalColor() const;
    QColor normalColor(const QString& typeId) const;
    QColor selectedColor() const;
    QColor selectedHaloColor() const;
    QColor hoveredColor() const;

    float lineWidth() const;
    float constructionLineWidth() const;
    float pointDiameter() const;

    bool useDataDefinedColors() const;

private:
    QColor ConstructionColor;
    QColor NormalColor;
    QColor SelectedColor;
    QColor SelectedHaloColor;
    QColor HoveredColor;

    float LineWidth;
    float ConstructionLineWidth;
    float PointDiameter;

    bool UseDataDefinedColors;

    QMap<QString, QColor> NormalColors;
};
} // namespace QtNodes
