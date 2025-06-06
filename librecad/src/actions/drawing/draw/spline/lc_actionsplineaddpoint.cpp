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
#include "lc_actionsplineaddpoint.h"

#include "lc_splinepoints.h"
#include "rs_entity.h"
#include "rs_spline.h"

namespace {
    const EntityTypeList g_enTypeList = {RS2::EntitySpline, RS2::EntitySplinePoints};
}

LC_ActionSplineAddPoint::LC_ActionSplineAddPoint(LC_ActionContext *actionContext)
   :LC_ActionSplineModifyBase("SplineAddPoint", actionContext, RS2::ActionDrawSplinePointAdd) {
}

void LC_ActionSplineAddPoint::doCompleteTrigger() {
    moveRelativeZero(m_vertexPoint);
}

void LC_ActionSplineAddPoint::doAfterTrigger() {
    m_directionFromStart = false;
    m_endpointIsSelected = false;
    setStatus(SetBeforeControlPoint);
}

void LC_ActionSplineAddPoint::onMouseMove(RS_Vector mouse, int status, LC_MouseEvent *e) {
    switch (status) {
        case SetEntity: {
            auto entity = catchEntityByEvent(e, g_enTypeList);
            if (entity != nullptr){
                if (mayModifySplineEntity(entity)) {
                    highlightHoverWithRefPoints(entity, true);
                }
            }
            break;
        }
        case SetBeforeControlPoint:{
            RS_Vector nearestRef = m_entityToModify->getNearestRef(mouse);
            if (nearestRef != m_entityToModify->getStartpoint()){
                RS_Vector nearestPointOnEntity = m_entityToModify->getNearestPointOnEntity(mouse, true);
                previewRefPoint(nearestPointOnEntity);
                previewRefSelectablePoint(nearestRef);
            }
            break;
        }
        case SetControlPoint:{
            previewRefSelectablePoint(mouse);
            RS_Vector nearestPointOnEntity = m_entityToModify->getNearestPointOnEntity(mouse, true);
            if (nearestPointOnEntity.valid){
                previewRefPoint(nearestPointOnEntity);
                previewRefPoint(m_selectedVertexPoint);
            }
            bool insertAfter = false;
            if (e->isShift){
                if (!m_endpointIsSelected){ // don't let inserting after endpoint
                    insertAfter = true;
                }
            }
            RS_Entity *previewUpdatedEntity = createModifiedSplineEntity(m_entityToModify, mouse, insertAfter);
            if (previewUpdatedEntity != nullptr) {
                previewEntity(previewUpdatedEntity);
            }
            break;
        }
        default:
            break;
    }
}

void LC_ActionSplineAddPoint::onMouseLeftButtonRelease(int status, LC_MouseEvent *e) {
    switch (status){
        case SetEntity:{
            auto entity = catchEntityByEvent(e, g_enTypeList);
            if (entity != nullptr && mayModifySplineEntity(entity)){
                m_entityToModify = entity;
                m_entityToModify->setSelected(true);
                redrawDrawing();
                setStatus(SetBeforeControlPoint);
            }
            break;
        }
        case SetBeforeControlPoint:{
            RS_Vector mouse = e->snapPoint;
            RS_Vector nearestRef = m_entityToModify->getNearestRef(mouse);
            if (nearestRef != m_entityToModify->getStartpoint()){
                m_selectedVertexPoint = nearestRef;
                moveRelativeZero(nearestRef);
                setStatus(SetControlPoint);
                m_endpointIsSelected = nearestRef == m_entityToModify->getEndpoint();
            }
            break;
        }
        case SetControlPoint:{
            if (e->isShift){
                if (!m_endpointIsSelected){ // don't let inserting after endpoint
                    m_directionFromStart = true;
                }
            }
            fireCoordinateEventForSnap(e);
        }
        default:
            break;
    }
}

RS_Entity *LC_ActionSplineAddPoint::createModifiedSplineEntity(RS_Entity *e, RS_Vector controlPoint, bool adjustPosition) {
    RS_Entity* result = nullptr;
    switch (e->rtti()){
        case RS2::EntitySplinePoints:{
            auto* splinePoints = dynamic_cast<LC_SplinePoints *>(e->clone());
            LC_SplinePointsData &data = splinePoints->getData();
            unsigned int controlPointsCount = data.controlPoints.size();
            unsigned int splinePointsCount = data.splinePoints.size();
            int insertIndexAdjustment = adjustPosition ? 1 : 0;
            if (splinePointsCount > 0){
                for (unsigned int i = 0; i < splinePointsCount; i++) {
                    RS_Vector cp = data.splinePoints.at(i);
                    if (cp == m_selectedVertexPoint) {
                        data.splinePoints.insert(data.splinePoints.begin()+ i + insertIndexAdjustment, controlPoint);
                        break;
                    }
                }
            }
            else{
                for (unsigned int i = 0; i < controlPointsCount; i++) {
                    RS_Vector cp = data.controlPoints.at(i);
                    if (cp == m_selectedVertexPoint) {
                        data.controlPoints.insert(data.controlPoints.begin()+ i + insertIndexAdjustment, controlPoint);
                        break;
                    }
                }
            }

            splinePoints->update();
            result = splinePoints;
            break;
        }
        case RS2::EntitySpline:{
            auto* spline = dynamic_cast<RS_Spline *>(e);
            RS_SplineData data = spline->getData();
            unsigned int controlPointsCount = data.controlPoints.size();
            int insertIndexAdjustment = adjustPosition ? 1 : 0;
            for (unsigned int i = 0; i < controlPointsCount; i++) {
                RS_Vector cp = data.controlPoints.at(i);
                if (cp == m_selectedVertexPoint) {
                    data.controlPoints.insert(data.controlPoints.begin()+ i + insertIndexAdjustment, controlPoint);
                    break;
                }
            }
            data.knotslist.clear();
            auto* modifiedSpline= new RS_Spline(nullptr, data);
            modifiedSpline->update();
            result = modifiedSpline;
            break;
        }
        case RS2::EntityParabola:{
            // fixme - sand - complete - there should be ordinary spline instead of parabola?
            break;
        }
        default:
            break;

    }
    return result;
}

void LC_ActionSplineAddPoint::onCoordinateEvent(int status, [[maybe_unused]]bool isZero, const RS_Vector &pos) {
    if (status == SetControlPoint){
        m_vertexPoint = pos;
        trigger();
    }
}

void LC_ActionSplineAddPoint::updateMouseButtonHints() {
    switch (getStatus()){
        case SetEntity:{
            updateMouseWidgetTRCancel(tr("Select spline or spline points entity"));
            break;
        }
        case SetBeforeControlPoint: {
            updateMouseWidgetTRCancel(tr("Select control point to insert before"));
            break;
        }
        case SetControlPoint: {
            updateMouseWidgetTRCancel(tr("Specify control point"), m_endpointIsSelected ? MOD_NONE : MOD_SHIFT_LC(tr("Insert AFTER selected point")));
            break;
        }
        default:
            break;
    }
}
