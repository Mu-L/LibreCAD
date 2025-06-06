/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2024 sand1024
**
** This file is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
**********************************************************************/
#ifndef LC_PENINFOREGISTRY_H
#define LC_PENINFOREGISTRY_H
#include <QIcon>
#include <QMap>
#include <qcoreapplication.h>

#include "rs.h"

class QRgba64;
class RS_Color;
/**
 * Registry that is used for resolving attributes of pens (line width, color, line type) and show them in
 * UI.
 *
 * Todo - in general, this registry may be also used for initialization of corresponding comboboxes
 * that will let us keep all intitialization in one place
 */
class LC_PenInfoRegistry{
    Q_DECLARE_TR_FUNCTIONS(LC_PenInfoRegistry)
public:
    enum ColorNameDisplayMode{
        HEX,
        RGB,
        NATURAL
    };

    LC_PenInfoRegistry();
    ~LC_PenInfoRegistry();

    QIcon getLineTypeIcon(RS2::LineType lineType) const;
    QIcon getLineWidthIcon(RS2::LineWidth lineWidth) const;
    QString getLineTypeText(RS2::LineType lineType) const;
    QString getLineWidthText(RS2::LineWidth lineWidth) const;
    QIcon getColorIcon(const RS_Color &color, int iconSizeW, int iconSizeH) const;
    QString getColorName(const RS_Color &color, int type) const;

    QString getInternalColorString(const RS_Color &color);
    RS_Color getColorFromInternalString(QString &str);

    static bool isUnchangedColor(const RS_Color &color);

    static LC_PenInfoRegistry* instance();
    static RS_Color createUnchangedColor();
    bool hasLineType(int typeCandidate);
    bool hasLineWidth(int widthCandidate);
private:

    QMap<RS2::LineType, QIcon> m_lineTypesIconMap;
    QMap<RS2::LineType, QString> m_lineTypesNamesMap;
    QMap<RS2::LineWidth, QIcon>m_lineWidthIconMap;
    QMap<RS2::LineWidth, QString> m_lineWidthNamesMap;
    QMap<QRgba64, QString> m_colorNamesMap;
    QMap<QRgba64, QString> m_standardLCColorNamesMap;

    QIcon m_iconByLayer = QIcon(":/icons/point_blank_square.lci");
    QIcon m_iconByBlock = QIcon(":/icons/point_plus_square.lci");
    QIcon m_iconUnchanged = QIcon(":/icons/point_cross.lci");

    void registerLineTypes();
    void registerLineWidths();
    void doRegisterLineType(const char* iconName, QString labelKey, RS2::LineType lineType);
    void doRegisterLineWidth(const char* iconName, QString labelKey, RS2::LineWidth lineWidth);
    void registerColorNames();
    void registerLCColor(QColor color, QString name);
};

#endif // LC_PENINFOREGISTRY_H
