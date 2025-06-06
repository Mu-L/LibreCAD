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

#include "rs_actionblocksadd.h"

#include "rs_block.h"
#include "rs_debug.h"
#include "rs_dialogfactory.h"
#include "rs_dialogfactoryinterface.h"
#include "rs_graphic.h"

RS_ActionBlocksAdd::RS_ActionBlocksAdd(LC_ActionContext *actionContext)
		:RS_ActionInterface("Add Block", actionContext, RS2::ActionBlocksAdd) {
	m_actionType = RS2::ActionBlocksAdd;
}

void RS_ActionBlocksAdd::trigger(){
    RS_DEBUG->print("adding block");
    if (m_graphic != nullptr){
        RS_BlockList *blockList = m_graphic->getBlockList();
        if (blockList){
            RS_BlockData d = RS_DIALOGFACTORY->requestNewBlockDialog(blockList);
            if (d.isValid()){
                // Block cannot contain blocks.
                if (m_container->is(RS2::EntityBlock)){
                    m_graphic->addBlock(new RS_Block(m_container->getParent(), d));
                } else {
                    m_graphic->addBlock(new RS_Block(m_container, d));
                }
            }
        }
    }
    finish(false);
}

void RS_ActionBlocksAdd::init(int status) {
    RS_ActionInterface::init(status);
    trigger();
}
