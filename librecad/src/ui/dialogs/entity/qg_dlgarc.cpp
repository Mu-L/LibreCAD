/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software 
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
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
#include "qg_dlgarc.h"

#include "rs_arc.h"
#include "rs_graphic.h"
#include "rs_math.h"
#include "rs_settings.h"

/*
 *  Constructs a QG_DlgArc as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
QG_DlgArc::QG_DlgArc(QWidget* parent)
    : LC_Dialog(parent, "ArcProperties"){
    setupUi(this);
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void QG_DlgArc::languageChange(){
    retranslateUi(this);
}

void QG_DlgArc::setArc(RS_Arc& a) {
    arc = &a;
    RS_Graphic* graphic = arc->getGraphic();
    if (graphic) {
        cbLayer->init(*(graphic->getLayerList()), false, false);
    }
    RS_Layer* lay = arc->getLayer(false);
    if (lay) {
        cbLayer->setLayer(*lay);
    }

    wPen->setPen(arc, lay, "Pen");

    leCenterX->setText(asString(arc->getCenter().x));
    leCenterY->setText(asString(arc->getCenter().y));
    leRadius->setText(asString(arc->getRadius()));
    leAngle1->setText(asStringAngleDeg(arc->getAngle1()));
    leAngle2->setText(asStringAngleDeg(arc->getAngle2()));

    cbReversed->setChecked(arc->isReversed());
    // fixme - sand - refactor to common function
    if (LC_GET_ONE_BOOL("Appearance","ShowEntityIDs", false)){
        lId->setText(QString("ID: %1").arg(arc->getId()));
    }
    else{
        lId->setVisible(false);
    }
}

void QG_DlgArc::updateArc() {
    arc->setCenter(RS_Vector(RS_Math::eval(leCenterX->text()),
                             RS_Math::eval(leCenterY->text())));
    arc->setRadius(RS_Math::eval(leRadius->text()));
    arc->setAngle1(RS_Math::deg2rad(RS_Math::eval(leAngle1->text())));
    arc->setAngle2(RS_Math::deg2rad(RS_Math::eval(leAngle2->text())));
    if (arc->isReversed() != cbReversed->isChecked()) {
        arc->revertDirection();
    }
    arc->setPen(wPen->getPen());
    arc->setLayer(cbLayer->currentText());
    arc->calculateBorders();
}
