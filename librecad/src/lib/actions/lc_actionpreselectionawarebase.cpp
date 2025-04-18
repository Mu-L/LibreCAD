/*******************************************************************************
 *
 This file is part of the LibreCAD project, a 2D CAD program

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
 ******************************************************************************/
#include <QMouseEvent>

#include "lc_actionpreselectionawarebase.h"
#include "rs_document.h"
#include "rs_graphicview.h"
#include "rs_debug.h"
#include "rs_settings.h"
#include "rs_selection.h"
#include "rs_overlaybox.h"

LC_ActionPreSelectionAwareBase::LC_ActionPreSelectionAwareBase(
    const char *name, RS_EntityContainer &container, RS_GraphicView &graphicView,
    const QList<RS2::EntityType> &entityTypeList, bool countSelectionDeep, RS2::ActionType actionType)
    :RS_ActionSelectBase(name, container, graphicView, entityTypeList, actionType),
    countDeep(countSelectionDeep)
{}

void LC_ActionPreSelectionAwareBase::doTrigger() {
    bool keepSelected = LC_GET_ONE_BOOL("Modify", "KeepModifiedSelected", true);
    doTrigger(keepSelected);
    updateMouseButtonHints(); // todo - is it really necessary??
}

LC_ActionPreSelectionAwareBase::~LC_ActionPreSelectionAwareBase() {
    selectedEntities.clear();
}

void LC_ActionPreSelectionAwareBase::init(int status) {
    RS_PreviewActionInterface::init(status);
    if (status < 0){
        selectedEntities.clear();
    }
    else{
        if (!selectionComplete) {
            unsigned int selectedCount = countSelectedEntities();
            if (selectedCount > 0) {
                selectionCompleted(false, true);
            }
        }
    }
}

unsigned int LC_ActionPreSelectionAwareBase::countSelectedEntities() {
    selectedEntities.clear();
    document->collectSelected(selectedEntities, countDeep, catchForSelectionEntityTypes);
    unsigned int selectedCount = selectedEntities.size();
//    LC_ERR << " Selected Count: " << selectedCount;
    return selectedCount;
}

void LC_ActionPreSelectionAwareBase::selectionFinishedByKey([[maybe_unused]]QKeyEvent *e, bool escape) {
    if (escape){
        selectedEntities.clear();
        finish(false);
    }
    else{
        if (!selectionComplete) {
            selectionCompleted(false,false);
        }
    }
}

void LC_ActionPreSelectionAwareBase::mousePressEvent(QMouseEvent * e) {
    if (!selectionComplete){
        if (e->button() == Qt::LeftButton){
            selectionCorner1 = toGraph(e);
        }
    }
}

void LC_ActionPreSelectionAwareBase::onMouseLeftButtonRelease(int status, LC_MouseEvent *e) {
    if (selectionComplete){
        mouseLeftButtonReleaseEventSelected(status, e);
    }
    else{
        if (inBoxSelectionMode){
            RS_Vector mouse = e->graphPoint;
            deletePreview();

            // restore selection box to ucs
            RS_Vector ucsP1 = toUCS(selectionCorner1);
            RS_Vector ucsP2 = toUCS(mouse);

            bool cross = (ucsP1.x > ucsP2.x);

            RS_Selection s(*container, viewport);
            bool select = !e->isShift;

            // expand selection wcs to ensure that selection box in ucs is full within bounding rect in wcs
            RS_Vector wcsP1, wcsP2;
            viewport->worldBoundingBox(ucsP1, ucsP2, wcsP1, wcsP2);

            if (catchForSelectionEntityTypes.isEmpty()){
                s.selectWindow(RS2::EntityUnknown, wcsP1, wcsP2, select, cross);
            }
            else {
                s.selectWindow(catchForSelectionEntityTypes, wcsP1, wcsP2, select, cross);
            }
            updateSelectionWidget();
        }
        else{
            RS_Entity* entityToSelect = catchEntityByEvent(e, catchForSelectionEntityTypes);
            bool selectContour = e->isShift;
            if (selectEntity(entityToSelect, selectContour)) {
                if (e->isControl) {
                    selectionCompleted(true, false);
                }
            }
        }
        inBoxSelectionMode = false;
        selectionCorner1.valid = false;
        invalidateSnapSpot();
    }
}

void LC_ActionPreSelectionAwareBase::onMouseRightButtonRelease(int status, LC_MouseEvent *e) {
    if (selectionComplete) {
        mouseRightButtonReleaseEventSelected(status, e);
    }
    else{
        selectedEntities.clear();
        finish(false);
    }
}

void LC_ActionPreSelectionAwareBase::onMouseMoveEvent(int status, LC_MouseEvent *e) {
    if (selectionComplete){
        onMouseMoveEventSelected(status, e);
    }
    else{
        RS_Vector mouse = e->graphPoint;
        if (selectionCorner1.valid && (viewport->toGuiDX(selectionCorner1.distanceTo(mouse)) > 10.0)){
            inBoxSelectionMode = true;
        }
        if (inBoxSelectionMode){
            drawOverlayBox(selectionCorner1, mouse);
            if (infoCursorOverlayPrefs->enabled) {
                // restore selection box to ucs
                RS_Vector ucsP1 = toUCS(selectionCorner1);
                RS_Vector ucsP2 = toUCS(mouse);
                bool cross = (ucsP1.x > ucsP2.x);
                bool deselect = e->isShift;
                QString msg = deselect ? tr("De-Selecting") : tr("Selecting");
                msg.append(tr(" entities "));
                msg.append(cross? tr("that intersect with box") : tr("that are within box"));
                infoCursorOverlayData.setZone2(msg);
                RS_Snapper::forceUpdateInfoCursor(mouse);
            }
        }
        else {
            selectionMouseMove(e);
            finishMouseMoveOnSelection(e);
        }
    }
}

void LC_ActionPreSelectionAwareBase::drawSnapper() {
    if (selectionComplete) {
        RS_Snapper::drawSnapper();
    }
}

void LC_ActionPreSelectionAwareBase::updateMouseButtonHints() {
    if (selectionComplete){
        updateMouseButtonHintsForSelected(getStatus());
    }
    else{
        if (inBoxSelectionMode){
            updateMouseWidgetTRBack(tr("Choose second edge"), MOD_SHIFT_LC(tr("Deselect entities")));
        }
        else {
            updateMouseButtonHintsForSelection();
        }
    }
}

void LC_ActionPreSelectionAwareBase::selectionCompleted([[maybe_unused]]bool singleEntity, bool fromInit) {
    setSelectionComplete(isAllowTriggerOnEmptySelection(), fromInit);
    updateMouseButtonHints();
    if (selectionComplete) {
        trigger();
        if (singleEntity) {
            deselectAll();
            selectionComplete = false; // continue with selection, don't finish
        } else {
            setStatus(-1);
        }
        updateSelectionWidget();
    }
}

void LC_ActionPreSelectionAwareBase::setSelectionComplete(bool allowEmptySelection, bool fromInit) {
    unsigned int selectedCount;
    if (fromInit) {
       selectedCount = selectedEntities.size();
    }
    else{
        selectedCount = countSelectedEntities();
    }
    bool proceed = selectedCount > 0 || allowEmptySelection;
    if (proceed) {
        selectionComplete = true;
        updateMouseButtonHintsForSelected(getStatus());
    }
    else{
        commandMessage(tr("No valid entities selected, select them first"));
    }
}

void LC_ActionPreSelectionAwareBase::updateMouseButtonHintsForSelected([[maybe_unused]]int status) {
    updateMouseWidget();
}

void LC_ActionPreSelectionAwareBase::mouseLeftButtonReleaseEventSelected([[maybe_unused]]int status, [[maybe_unused]]LC_MouseEvent *pEvent) {}

void LC_ActionPreSelectionAwareBase::mouseRightButtonReleaseEventSelected([[maybe_unused]]int status, [[maybe_unused]]LC_MouseEvent *pEvent) {}

void LC_ActionPreSelectionAwareBase::onMouseMoveEventSelected([[maybe_unused]] int status, [[maybe_unused]]LC_MouseEvent *event) {}

RS2::CursorType LC_ActionPreSelectionAwareBase::doGetMouseCursor(int status) {
    if (selectionComplete){
        return doGetMouseCursorSelected(status);
    }
    else {
        return RS_ActionSelectBase::doGetMouseCursor(status);
    }
}

RS2::CursorType LC_ActionPreSelectionAwareBase::doGetMouseCursorSelected([[maybe_unused]]int status) {
    return RS2::CadCursor;
}

void LC_ActionPreSelectionAwareBase::finishMouseMoveOnSelection([[maybe_unused]]LC_MouseEvent *event) {

}

void LC_ActionPreSelectionAwareBase::doSelectEntity(RS_Entity *entityToSelect, bool selectContour) const {
    if (entityToSelect != nullptr){
        RS_Selection s(*container, viewport);
        // try to minimize selection clicks - and select contour based on selected entity. May be optional, but what for?
        if (entityToSelect->isAtomic() && selectContour) {
            s.selectContour(entityToSelect);
        }
        else{
            s.selectSingle(entityToSelect);
        }
    }
}
