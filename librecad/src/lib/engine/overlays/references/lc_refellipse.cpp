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

#include "lc_refellipse.h"

LC_RefEllipse::LC_RefEllipse(RS_EntityContainer *parent, const RS_EllipseData &d):RS_Ellipse(parent, d) {}

RS2::EntityType LC_RefEllipse::rtti() const{
    return RS2::EntityRefEllipse;
}

RS_Entity *LC_RefEllipse::clone() const{
    auto* a = new LC_RefEllipse(*this);
    return a;
}
