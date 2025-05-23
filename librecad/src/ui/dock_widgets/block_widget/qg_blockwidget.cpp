/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2020 A. Stebich (librecad@mail.lordofbikes.de)
** Copyright (C) 2011 Rallaz (rallazz@gmail.com)
** Copyright (C) 2010-2011 R. van Twisk (librecad@rvt.dds.nl)
**
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

#include <algorithm>

#include <QScrollBar>
#include <QTableView>
#include <QHeaderView>
#include <QToolButton>
#include <QMenu>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QContextMenuEvent>

#include "lc_flexlayout.h"
#include "qg_actionhandler.h"
#include "qg_blockwidget.h"

#include "lc_actiongroupmanager.h"
#include "rs_blocklist.h"
#include "rs_debug.h"
#include "rs_settings.h"
#include "lc_widgets_common.h"
#include "rs_graphic.h"
#include "rs_graphicview.h"

QG_BlockModel::QG_BlockModel(QObject * parent) : QAbstractTableModel(parent) {
    m_iconBlockVisible = QIcon(":/icons/visible.lci");
//    blockHidden = QIcon(":/icons/invisible.svg");
    m_iconBlockHidden = QIcon(":/icons/not_visible.lci");
}

int QG_BlockModel::rowCount ( const QModelIndex & /*parent*/ ) const {
    return m_listBlock.size();
}

QModelIndex QG_BlockModel::parent ( const QModelIndex & /*index*/ ) const {
    return QModelIndex();
}

QModelIndex QG_BlockModel::index ( int row, int column, const QModelIndex & /*parent*/ ) const {
    if ( row >= m_listBlock.size() || row < 0)
        return QModelIndex();
    return createIndex ( row, column);
}

bool blockLessThan(const RS_Block *s1, const RS_Block *s2) {
     return s1->getName() < s2->getName();
}

void QG_BlockModel::setBlockList(RS_BlockList* bl) {
    /* since 4.6 the recommended way is to use begin/endResetModel()
     * TNick <nicu.tofan@gmail.com>
     */
    beginResetModel();
    m_listBlock.clear();
    if (bl == nullptr){
        endResetModel();
        return;
    }
    for (int i=0; i<bl->count(); ++i) {
        if ( !bl->at(i)->isUndone() )
            m_listBlock.append(bl->at(i));
    }
    setActiveBlock(bl->getActive());
    std::sort( m_listBlock.begin(), m_listBlock.end(), blockLessThan);

    //called to force redraw
    endResetModel();
}


RS_Block *QG_BlockModel::getBlock( int row) const{
    if ( row >= m_listBlock.size() || row < 0)
        return nullptr;
    return m_listBlock.at(row);
}

QModelIndex QG_BlockModel::getIndex (RS_Block * blk) const{
    int row = m_listBlock.indexOf(blk);
    if (row<0)
        return QModelIndex();
    return createIndex ( row, NAME);
}

QVariant QG_BlockModel::data ( const QModelIndex & index, int role ) const {
    if (!index.isValid() || index.row() >= m_listBlock.size())
        return QVariant();

    RS_Block* blk = m_listBlock.at(index.row());

    if (role ==Qt::DecorationRole && index.column() == VISIBLE) {
        if (!blk->isFrozen()) {
            return m_iconBlockVisible;
        } else {
            return m_iconBlockHidden;
        }
    }
    if (role ==Qt::DisplayRole && index.column() == NAME) {
        return blk->getName();
    }
    if (role == Qt::FontRole && index.column() == NAME) {
        if (m_activeBlock && m_activeBlock == blk) {
            QFont font;
            font.setBold(true);
            return font;
        }
    }
//Other roles:
    return QVariant();
}

 /**
 * Constructor.
 */
QG_BlockWidget::QG_BlockWidget(LC_ActionGroupManager* agm,QG_ActionHandler* ah, QWidget* parent,
                               const char* name, Qt::WindowFlags f)
        : LC_GraphicViewAwareWidget(parent, name, f) {
    m_actionGroupManager = agm;
    m_actionHandler = ah;
    m_blockList = nullptr;
    m_lastBlock = nullptr;

    m_blockModel = new QG_BlockModel(this);
    m_blockView = new QTableView(this);
    m_blockView->setModel (m_blockModel);
    m_blockView->setShowGrid (false);
    m_blockView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_blockView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_blockView->setFocusPolicy(Qt::NoFocus);
    m_blockView->setColumnWidth(QG_BlockModel::VISIBLE, 20);
    m_blockView->verticalHeader()->hide();
    m_blockView->horizontalHeader()->setStretchLastSection(true);
    m_blockView->horizontalHeader()->hide();

#ifndef DONT_FORCE_WIDGETS_CSS
    blockView->setStyleSheet("QWidget {background-color: white;}  QScrollBar{ background-color: none }");
#endif
    auto* lay = new QVBoxLayout(this);
    lay->setSpacing ( 2 );
    lay->setContentsMargins(2, 2, 2, 2);

    auto layButtons = new LC_FlexLayout(0,5,5);

    addToolbarButton(layButtons, RS2::ActionBlocksDefreezeAll);
    addToolbarButton(layButtons, RS2::ActionBlocksFreezeAll);
    addToolbarButton(layButtons, RS2::ActionBlocksCreate);
    addToolbarButton(layButtons, RS2::ActionBlocksAdd);
    addToolbarButton(layButtons, RS2::ActionBlocksRemove);
    addToolbarButton(layButtons, RS2::ActionBlocksAttributes);
    addToolbarButton(layButtons, RS2::ActionBlocksEdit);
    addToolbarButton(layButtons, RS2::ActionBlocksSave);
    addToolbarButton(layButtons, RS2::ActionBlocksInsert);

    // lineEdit to filter block list with RegEx
    m_matchBlockName = new QLineEdit(this);
    m_matchBlockName->setReadOnly(false);
    m_matchBlockName->setPlaceholderText(tr("Filter"));
    m_matchBlockName->setClearButtonEnabled(true);
    m_matchBlockName->setToolTip(tr("Looking for matching block names"));
    connect(m_matchBlockName, &QLineEdit::textChanged, this, &QG_BlockWidget::slotUpdateBlockList);

    lay->addWidget(m_matchBlockName);
    lay->addLayout(layButtons);
    // lay->addLayout(layButtons2);
    lay->addWidget(m_blockView);

    connect(m_blockView, &QTableView::clicked, this, &QG_BlockWidget::slotActivated);
    connect(m_blockView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &QG_BlockWidget::slotSelectionChanged);

    updateWidgetSettings();
}

void QG_BlockWidget::addToolbarButton(LC_FlexLayout* layButtons, RS2::ActionType actionType) {
    QAction* action = m_actionGroupManager->getActionByType(actionType);
    if (action != nullptr) {
        auto button = new QToolButton(this);
        button->setDefaultAction(action);
        layButtons->addWidget(button);
    }
}


/**
 * Updates the block box from the blocks in the graphic.
 */
void QG_BlockWidget::update() {
    RS_DEBUG->print("QG_BlockWidget::update()");

    if (m_blockList==nullptr) {
        RS_DEBUG->print("QG_BlockWidget::update(): blockList is nullptr");
        m_blockModel->setActiveBlock(nullptr);
        m_blockModel->setBlockList(nullptr);
        m_lastBlock = nullptr;
        return;
    }

    RS_Block* activeBlock =m_blockList->getActive();

    m_blockModel->setBlockList(m_blockList);

    RS_Block* b = m_lastBlock;
    activateBlock(activeBlock);
    m_lastBlock = b;
    m_blockView->resizeRowsToContents();

    restoreSelections();

    RS_DEBUG->print("QG_BlockWidget::update() done");
}


void QG_BlockWidget::restoreSelections() const {
    QItemSelectionModel* selectionModel = m_blockView->selectionModel();

    for (auto block: *m_blockList) {
        if (block == nullptr || !block->isVisibleInBlockList() || !block->isSelectedInBlockList())
            continue;

        QModelIndex idx = m_blockModel->getIndex(block);
        QItemSelection selection(idx, idx);
        selectionModel->select(selection, QItemSelectionModel::Select);
    }
}


/**
 * Activates the given block and makes it the active
 * block in the blocklist.
 */
void QG_BlockWidget::activateBlock(RS_Block* block) {
    RS_DEBUG->print("QG_BlockWidget::activateBlock()");

    if (block==nullptr || m_blockList==nullptr) {
        return;
    }

    m_lastBlock = m_blockList->getActive();
    m_blockList->activate(block);
    int yPos = m_blockView->verticalScrollBar()->value();
    QModelIndex idx = m_blockModel->getIndex (block);

    // remember selected status of the block
    bool selected = block->isSelectedInBlockList();

    m_blockView->setCurrentIndex ( idx );
    m_blockModel->setActiveBlock(block);
    m_blockView->viewport()->update();

    // restore selected status of the block
    QItemSelectionModel::SelectionFlag selFlag = selected ? QItemSelectionModel::Select : QItemSelectionModel::Deselect;
    block->selectedInBlockList(selected);
    m_blockView->selectionModel()->select(QItemSelection(idx, idx), selFlag);
    m_blockView->verticalScrollBar()->setValue(yPos);
}

/**
 * Called when the user activates (highlights) a block.
 */
void QG_BlockWidget::slotActivated(const QModelIndex &blockIdx) {
    if (!blockIdx.isValid() || m_blockList==nullptr)
        return;

    RS_Block * block = m_blockModel->getBlock( blockIdx.row() );
    if (block == nullptr)
        return;

    if (blockIdx.column() == QG_BlockModel::VISIBLE) {
        RS_Block* b = m_blockList->getActive();
        m_blockList->activate(block);
        m_actionHandler->setCurrentAction(RS2::ActionBlocksToggleView);
        activateBlock(b);
        return;
    }

    if (blockIdx.column() == QG_BlockModel::NAME) {
        m_lastBlock = m_blockList->getActive();
        activateBlock(block);
    }
}


/**
 * Called on blocks selection/deselection
 */
void QG_BlockWidget::slotSelectionChanged(
    const QItemSelection &selected,
    const QItemSelection &deselected) const {
    QItemSelectionModel *selectionModel {m_blockView->selectionModel()};

    foreach (QModelIndex index, selected.indexes()) {
        auto block = m_blockModel->getBlock(index.row());
        if (block != nullptr) {
            block->selectedInBlockList(true);
            selectionModel->select(QItemSelection(index, index), QItemSelectionModel::Select);
        }
    }

    foreach (QModelIndex index, deselected.indexes()) {
        auto block = m_blockModel->getBlock(index.row());
        if (block != nullptr && block->isVisibleInBlockList()) {
            block->selectedInBlockList(false);
            selectionModel->select(QItemSelection(index, index), QItemSelectionModel::Deselect);
        }
    }
}


/**
 * Shows a context menu for the block widget. Launched with a right click.
 */
void QG_BlockWidget::contextMenuEvent(QContextMenuEvent *e) {
    // select item (block) in Block List widget first because left-mouse-click event are not to be triggered
    // slotActivated(blockView->currentIndex());
    auto contextMenu = std::make_unique<QMenu>(this);
    auto menu = contextMenu.get();
    // Actions for all blocks:
    addMenuItem(menu, RS2::ActionBlocksDefreezeAll);
    addMenuItem(menu, RS2::ActionBlocksFreezeAll);
    contextMenu->addSeparator();
    // Actions for selected blocks or, if nothing is selected, for active block:
    addMenuItem(menu, RS2::ActionBlocksToggleView);
    addMenuItem(menu, RS2::ActionBlocksRemove);
    contextMenu->addSeparator();
    // Single block actions:
    addMenuItem(menu, RS2::ActionBlocksAdd);
    addMenuItem(menu, RS2::ActionBlocksAttributes);
    addMenuItem(menu, RS2::ActionBlocksEdit);
    addMenuItem(menu, RS2::ActionBlocksInsert);
    addMenuItem(menu, RS2::ActionBlocksCreate);
    contextMenu->exec(QCursor::pos());
    e->accept();
}

void QG_BlockWidget::addMenuItem(QMenu* contextMenu, RS2::ActionType actionType) {
    auto action = m_actionGroupManager->getActionByType(actionType);
    if (action != nullptr) {
        contextMenu->addAction(action);
    }
}

/**
 * Escape releases focus.
 */
void QG_BlockWidget::keyPressEvent(QKeyEvent* e) {
    switch (e->key()) {
    case Qt::Key_Escape:
        emit escape();
        break;

    default:
        QWidget::keyPressEvent(e);
        break;
    }
}


void QG_BlockWidget::blockAdded(RS_Block*) {
    update();
    if (! m_matchBlockName->text().isEmpty()) {
        slotUpdateBlockList();
    }
}


/**
 * Called when reg-expression matchBlockName->text changed
 */
void QG_BlockWidget::slotUpdateBlockList() const {
    if (m_blockList == nullptr) {
        return;
    }

    QRegularExpression rx{ QRegularExpression::wildcardToRegularExpression(m_matchBlockName->text())};

    for (int i = 0; i < m_blockList->count(); i++) {
        RS_Block* block = m_blockModel->getBlock(i);
        if (!block) continue;
        if (block->getName().indexOf(rx) == 0) {
            m_blockView->showRow(i);
            m_blockModel->getBlock(i)->visibleInBlockList(true);
        } else {
            m_blockView->hideRow(i);
            m_blockModel->getBlock(i)->visibleInBlockList(false);
        }
    }

    restoreSelections();
}

void QG_BlockWidget::updateWidgetSettings() const {
    LC_GROUP("Widgets"); {
        bool flatIcons = LC_GET_BOOL("DockWidgetsFlatIcons", true);
        int iconSize = LC_GET_INT("DockWidgetsIconSize", 16);

        QSize size(iconSize, iconSize);

        QList<QToolButton *> widgets = this->findChildren<QToolButton *>();
        foreach(QToolButton *w, widgets) {
            w->setAutoRaise(flatIcons);
            w->setIconSize(size);
        }
    }
    LC_GROUP_END();
}

void QG_BlockWidget::setGraphicView(RS_GraphicView* gv){
    if (gv == nullptr) {
        setBlockList(nullptr);
    }
    else {
        auto graphic = gv->getGraphic();
        if (graphic == nullptr) {
            setBlockList(nullptr);
        }
        else {
            setBlockList(graphic->getBlockList());
        }
    }
}

void QG_BlockWidget::setBlockList(RS_BlockList* blockList) {
    if (m_blockList != nullptr) {
        m_blockList->removeListener(this);
    }
    m_blockList = blockList;
    if (blockList != nullptr) {
        m_blockList->addListener(this);
    }
    update();
}
