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

#ifndef RS_ACTIONDRAWCIRCLE2P_H
#define RS_ACTIONDRAWCIRCLE2P_H

#include "lc_actiondrawcirclebase.h"

struct RS_CircleData;

/**
 * This action class can handle user events to draw 
 * simple arcs with the center, radius, start- and endangle given.
 *
 * @author Andrew Mustun
 */
class RS_ActionDrawCircle2P:public LC_ActionDrawCircleBase {
    Q_OBJECT
public:
    RS_ActionDrawCircle2P(LC_ActionContext *actionContext);
    ~RS_ActionDrawCircle2P() override;
    void reset() override;
    void preparePreview();
protected:
    /**
 * Action States.
 */
    enum Status {
        SetPoint1,       /**< Setting the 1st point. */
        SetPoint2        /**< Setting the 2nd point. */
    };

    /**
     * Circle data defined so far.
     */
    std::unique_ptr<RS_CircleData> m_circleData;
    struct Points;
    std::unique_ptr<Points> m_actionData;
    void onMouseLeftButtonRelease(int status, LC_MouseEvent *e) override;
    void onMouseRightButtonRelease(int status, LC_MouseEvent *e) override;
    void onCoordinateEvent(int status, bool isZero, const RS_Vector &pos) override;
    void updateMouseButtonHints() override;
    void doTrigger() override;
    void onMouseMoveEvent(int status, LC_MouseEvent *event) override;
};
#endif
