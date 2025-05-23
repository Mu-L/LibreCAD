/****************************************************************************
**
* Options widget for "DrawStar" action.

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
#include "lc_staroptions.h"
#include "lc_actiondrawstar.h"
#include "ui_lc_staroptions.h"

LC_StarOptions::LC_StarOptions() :
    LC_ActionOptionsWidgetBase(RS2::ActionDrawStar, "Draw","Star"),
    ui(new Ui::LC_StarOptions),
    m_action(nullptr){
    ui->setupUi(this);

    ui->sbNumber->setMaximum(STAR_MIN_RAYS);
    ui->sbNumber->setMaximum(STAR_MAX_RAYS);

    connect(ui->cbSymmertix, &QCheckBox::clicked, this, &LC_StarOptions::onSymmetricClicked);
    connect(ui->cbRadiusInner, &QCheckBox::clicked, this, &LC_StarOptions::onRadiusInnerClicked);
    connect(ui->leRadiusInner, &QLineEdit::editingFinished, this, &LC_StarOptions::onRadiusInnerEditingFinished);
    connect(ui->cbRadiusOuter, &QCheckBox::clicked, this, &LC_StarOptions::onRadiusOuterClicked);
    connect(ui->leRadusOuter, &QLineEdit::editingFinished, this, &LC_StarOptions::onRadiusOuterEditingFinished);
    connect(ui->cbPolyline, &QCheckBox::clicked, this, &LC_StarOptions::onPolylineClicked);
    connect(ui->sbNumber, &QSpinBox::valueChanged, this, &LC_StarOptions::onNumberChanged);
}

LC_StarOptions::~LC_StarOptions(){
    m_action = nullptr;
    delete ui;
}

void LC_StarOptions::languageChange() {
    ui->retranslateUi(this);
}

void LC_StarOptions::doSetAction(RS_ActionInterface *a, bool update){
    m_action = dynamic_cast<LC_ActionDrawStar *>(a);

    bool polyline;
    bool innerEnabled;
    bool outerEnabled;
    int number;
    bool symmetric;
    QString innerRadius;
    QString outerRadius;
    if (update){
        polyline = m_action->isPolyline();
        innerEnabled = m_action->isInnerRounded();
        outerEnabled = m_action->isOuterRounded();
        number = m_action ->getRaysNumber();
        symmetric = m_action->isSymmetric();
        innerRadius = fromDouble(m_action->getRadiusInner());
        outerRadius = fromDouble(m_action->getRadiusOuter());
    }
    else{     
        number = loadInt("Number", 5);
        innerEnabled = loadBool("InnerRound", false);
        innerRadius = load("InnerRadius", "1");
        outerEnabled = loadBool("OuterRound", false);
        outerRadius = load("OuterRadius", "1");
        symmetric = loadBool("Symmetric", true) ;
        polyline = loadBool("Polyline", true);
    }

    setNumberToModelAndView(number);
    setRadiusInnerEnabledToModelAndView(innerEnabled);
    setRadiusInnerToModelAndView(innerRadius);
    setRadiusOuterEnabledToModelAndView(outerEnabled);
    setRadiusOuterToModelAndView(outerRadius);
    setSymmetricToModelAndView(symmetric);
    setUsePolylineToActionAndView(polyline);
}

void LC_StarOptions::doSaveSettings(){    
    save("Number", ui->sbNumber->value());
    save("InnerRound", ui->cbRadiusInner->isChecked());
    save("InnerRadius", ui->leRadiusInner->text());
    save("OuterRound", ui->cbRadiusOuter->isChecked());
    save("OuterRadius", ui->leRadusOuter->text());
    save("Symmetric", ui->cbSymmertix->isChecked());
    save("Polyline", ui->cbPolyline->isChecked());
}

void LC_StarOptions::onSymmetricClicked(bool value){
    if (m_action != nullptr){
        setSymmetricToModelAndView(value);
    }
}

void LC_StarOptions::onRadiusOuterEditingFinished(){
    if (m_action != nullptr){
        setRadiusOuterToModelAndView(ui->leRadusOuter->text());
    }
}

void LC_StarOptions::onRadiusInnerEditingFinished(){
    if (m_action != nullptr){
        setRadiusInnerToModelAndView(ui->leRadiusInner->text());
    }
}

void LC_StarOptions::onRadiusInnerClicked(bool value){
    if (m_action != nullptr){
        setRadiusInnerEnabledToModelAndView(value);
    }
}

void LC_StarOptions::onRadiusOuterClicked(bool value){
    if (m_action != nullptr){
        setRadiusOuterEnabledToModelAndView(value);
    }
}

void LC_StarOptions::onNumberChanged(int value){
    if (m_action != nullptr){
        setNumberToModelAndView(value);
    }
}

void LC_StarOptions::setRadiusOuterToModelAndView(const QString& value){
    double y;
    if (toDouble(value, y, 0.0, true)){
        m_action->setRadiusOuter(y);
        ui->leRadusOuter->setText(fromDouble(y));
    }
}

void LC_StarOptions::setRadiusInnerToModelAndView(const QString& value){
    double y;
    if (toDouble(value, y, 0.0, true)){
        m_action->setRadiusInner(y);
        ui->leRadiusInner->setText(fromDouble(y));
    }
}

void LC_StarOptions::setRadiusInnerEnabledToModelAndView(bool value){
    m_action->setInnerRounded(value);
    ui->cbRadiusInner ->setChecked(value);
    ui->leRadiusInner->setEnabled(value);
}

void LC_StarOptions::setSymmetricToModelAndView(bool value){
    m_action->setSymmetric(value);
    ui->cbSymmertix->setChecked(value);
}

void LC_StarOptions::onPolylineClicked(bool value){
    if (m_action != nullptr){
        setUsePolylineToActionAndView(value);
    }
}

void LC_StarOptions::setRadiusOuterEnabledToModelAndView(bool value){
    m_action->setOuterRounded(value);
    ui->cbRadiusOuter->setChecked(value);
    ui->leRadusOuter->setEnabled(value);
}

void LC_StarOptions::setNumberToModelAndView(int value){
    m_action->setRaysNumber(value);
    ui->sbNumber->setValue(value);
}

void LC_StarOptions::setUsePolylineToActionAndView(bool value){
    m_action->setPolyline(value);
    ui->cbPolyline->setChecked(value);
}
