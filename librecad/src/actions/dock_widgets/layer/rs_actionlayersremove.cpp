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

#include "rs_actionlayersremove.h"

#include "rs_debug.h"
#include "rs_dialogfactory.h"
#include "rs_dialogfactoryinterface.h"
#include "rs_graphic.h"

RS_ActionLayersRemove::RS_ActionLayersRemove(LC_ActionContext *actionContext)
        :RS_ActionInterface("Remove Layer", actionContext, RS2::ActionLayersRemove) {}


void RS_ActionLayersRemove::trigger() {
    RS_DEBUG->print("RS_ActionLayersRemove::trigger");

    if (m_graphic) {
        RS_LayerList *ll = m_graphic->getLayerList();
        QStringList names =
            RS_DIALOGFACTORY->requestSelectedLayersRemovalDialog(ll);

        if (!names.isEmpty()) {
            for (auto name: names) {
                m_graphic->removeLayer(ll->find(name));
            }
            m_graphic->updateInserts();
            m_container->calculateBorders();
            // m_graphic->getLayerList()->getLayerWitget()->slotUpdateLayerList();
        }
    }
    finish(false);
    updateSelectionWidget();
}

void RS_ActionLayersRemove::init(int status) {
    RS_ActionInterface::init(status);
    trigger();
}
