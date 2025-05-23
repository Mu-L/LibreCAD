/****************************************************************************
**
* Options widget for "LineFromPointToLine" action.

Copyright (C) 2024 LibreCAD.org
Copyright (C) 2024 sand1024

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**********************************************************************/

#include "lc_linefrompointtolineoptions.h"
#include "lc_actiondrawlinefrompointtoline.h"
#include "ui_lc_linefrompointtolineoptions.h"


LC_LineFromPointToLineOptions::LC_LineFromPointToLineOptions() :
    LC_ActionOptionsWidgetBase(RS2::ActionDrawLineFromPointToLine, "Draw", "LinePointToLine"),
    m_action(nullptr),
    ui(new Ui::LC_LineFromPointToLineOptions){
    ui->setupUi(this);

    connect(ui->cbOrthogonal, &QCheckBox::clicked, this, &LC_LineFromPointToLineOptions::onOrthogonalClicked);
    connect(ui->leAngle, &QLineEdit::editingFinished, this, &LC_LineFromPointToLineOptions::onAngleEditingFinished);
    connect(ui->cbSizeMode, &QComboBox::currentIndexChanged, this, &LC_LineFromPointToLineOptions::onSizeModeIndexChanged);
    connect(ui->leLength, &QLineEdit::editingFinished, this, &LC_LineFromPointToLineOptions::onLengthEditingFinished);
    connect(ui->leOffset, &QLineEdit::editingFinished, this, &LC_LineFromPointToLineOptions::onEndOffsetEditingFinished);
    connect(ui->cbSnap, &QComboBox::currentIndexChanged,this, &LC_LineFromPointToLineOptions::onSnapModeIndexChanged);
}

LC_LineFromPointToLineOptions::~LC_LineFromPointToLineOptions(){
    delete ui;
    m_action = nullptr;
}

void LC_LineFromPointToLineOptions::languageChange(){
    ui->retranslateUi(this);
}

void LC_LineFromPointToLineOptions::doSaveSettings(){
    save("Orthogonal", ui->cbOrthogonal->isChecked());
    save("Angle", ui->leAngle->text());
    save("Length", ui->leLength->text());
    save("SnapMode", ui->cbSnap->currentIndex());
    save("SizeMode", ui->cbSizeMode->currentIndex());
    save("EndOffset", ui->leOffset->text());
}

void LC_LineFromPointToLineOptions::doSetAction(RS_ActionInterface *a, bool update){
    m_action = dynamic_cast<LC_ActionDrawLineFromPointToLine *>(a);
    bool orthogonal;
    QString angle;
    int sizeMode;
    QString length;
    QString offset;
    int snap;

    if (update){
        orthogonal = m_action->getOrthogonal();
        sizeMode = m_action->getSizeMode();
        snap = m_action->getLineSnapMode();
        angle = fromDouble(m_action->getAngle());
        length = fromDouble(m_action->getLength());
        offset = fromDouble(m_action->getEndOffset());
    }
    else{
        orthogonal = loadBool("Orthogonal", true) ;
        angle = load("Angle", "90");
        length = load("Length", "100");
        snap = loadInt("SnapMode", 0);
        sizeMode = loadInt("SizeMode", 0);
        offset = load("EndOffset", "0.0");
    }
    setOrthogonalToActionAndView(orthogonal);
    setAngleToActionAndView(angle);
    setSizeModelIndexToActionAndView(sizeMode);
    setLengthToActionAndView(length);
    setSnapModeToActionAndView(snap);
    setEndOffsetToActionAndView(offset);
}

void LC_LineFromPointToLineOptions::onSnapModeIndexChanged(int index){
    if (m_action != nullptr){
        setSnapModeToActionAndView(index);
    }
}

void LC_LineFromPointToLineOptions::onSizeModeIndexChanged(int index){
    if (m_action != nullptr){
        setSizeModelIndexToActionAndView(index);
    }
}

void LC_LineFromPointToLineOptions::onOrthogonalClicked(bool value){
    if (m_action != nullptr){
        setOrthogonalToActionAndView(value);
    }
}

void LC_LineFromPointToLineOptions::onAngleEditingFinished(){
    if (m_action != nullptr){
        setAngleToActionAndView(ui->leAngle->text());
    }
}

void LC_LineFromPointToLineOptions::onLengthEditingFinished(){
    if (m_action != nullptr){
        setLengthToActionAndView(ui->leLength->text());
    }
}
void LC_LineFromPointToLineOptions::onEndOffsetEditingFinished(){
    if (m_action != nullptr){
        setEndOffsetToActionAndView(ui->leOffset->text());
    }
}

void LC_LineFromPointToLineOptions::setSnapModeToActionAndView(int index){
    m_action->setLineSnapMode(index);
    ui->cbSnap->setCurrentIndex(index);
}

void LC_LineFromPointToLineOptions::setSizeModelIndexToActionAndView(int index){
    m_action->setSizeMode(index);
    ui->cbSizeMode->setCurrentIndex(index);
    bool intersectionMode = index == 0;
    ui->frmLength->setVisible(!intersectionMode);
    ui->frmOffset->setVisible(intersectionMode);
}

void LC_LineFromPointToLineOptions::setAngleToActionAndView(const QString& value){
    double angle;
    if (toDoubleAngleDegrees(value, angle, 1.0, false)){
        // ensure angle in 0..180
        double angleRad = RS_Math::deg2rad(angle);
        double correctedAngle = std::remainder(angleRad, M_PI);
        angle = RS_Math::rad2deg(std::abs(correctedAngle));

        m_action->setAngle(angle);
        ui->leAngle->setText(fromDouble(angle));
    }
}

void LC_LineFromPointToLineOptions::setLengthToActionAndView(const QString& value){
    double len;
    if (toDouble(value, len, 1.0, false)){
        m_action->setLength(len);
        ui->leLength->setText(fromDouble(len));
    }
}

void LC_LineFromPointToLineOptions::setEndOffsetToActionAndView(const QString& value){
    double len;
    if (toDouble(value, len, 0.0, false)){
        m_action->setEndOffset(len);
        ui->leOffset->setText(fromDouble(len));
    }
}

void LC_LineFromPointToLineOptions::setOrthogonalToActionAndView(bool value){
    m_action->setOrthogonal(value);
    ui->cbOrthogonal->setChecked(value);

    ui->lblAngle->setEnabled(!value);
    ui->leAngle->setEnabled(!value);
}
