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

#ifndef RS_ACTIONSELECTINTERSECTED_H
#define RS_ACTIONSELECTINTERSECTED_H

#include "rs_previewactioninterface.h"

/**
 * This action class can handle user events to select all entities.
 *
 * @author Andrew Mustun
 */
class RS_ActionSelectIntersected:public RS_PreviewActionInterface {
    Q_OBJECT
public:
    RS_ActionSelectIntersected(LC_ActionContext *actionContext,bool select);
    ~RS_ActionSelectIntersected() override;
    void init(int status) override;
protected:
    /**
     * Action States.
     */
    enum Status {
        SetPoint1,     /**< Setting the 1st corner of the window.  */
        SetPoint2      /**< Setting the 2nd corner of the window. */
    };

    struct ActionData;
    std::unique_ptr<ActionData> m_actionData;
    bool m_performSelect = false;
    RS2::CursorType doGetMouseCursor(int status) override;
    void onMouseLeftButtonRelease(int status, LC_MouseEvent *e) override;
    void onMouseRightButtonRelease(int status, LC_MouseEvent *e) override;
    void onMouseMoveEvent(int status, LC_MouseEvent *event) override;
    void onMouseLeftButtonPress(int status, LC_MouseEvent *e) override;
    void updateMouseButtonHints() override;
    void doTrigger() override;
};
#endif
