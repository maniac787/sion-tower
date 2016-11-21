/*
 * This file is part of SionTower.
 *
 * 
 * David Saltares M&aacute;rquez (C) 2011
 * <david.saltares@gmail.com>
 *
 * 
 * SionTower examples are free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License ad
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) ant later version.
 *
 * SionTower examples are distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SionTower examples.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>

#include "cell.h"

using std::cout;
using std::cerr;
using std::endl;


Cell::Cell(int id,
           const Ogre::Vector3& pointA,
           const Ogre::Vector3& pointB,
           const Ogre::Vector3& pointC): _id(id) {

    // Inicializamos los vértices
    _vertex[VERT_A] = pointA;
    _vertex[VERT_B] = pointB;
    _vertex[VERT_C] = pointC;
    
    // No contamos con celdas vecinas inicialmente
    _links[SIDE_AB] = 0;
    _links[SIDE_BC] = 0;
    _links[SIDE_CA] = 0;
    
    // Calculamos el centro
    _center.x = (_vertex[VERT_A].x + _vertex[VERT_B].x + _vertex[VERT_C].x) / 3.0f;
    _center.y = (_vertex[VERT_A].y + _vertex[VERT_B].y + _vertex[VERT_C].y) / 3.0f;
    _center.z = (_vertex[VERT_A].z + _vertex[VERT_B].z + _vertex[VERT_C].z) / 3.0f;
    
    // Creamos vectores 2D
    Ogre::Vector2 p1(_vertex[VERT_A].x, _vertex[VERT_A].z);
    Ogre::Vector2 p2(_vertex[VERT_B].x, _vertex[VERT_B].z);
    Ogre::Vector2 p3(_vertex[VERT_C].x, _vertex[VERT_C].z);
    
    _sides[SIDE_AB].setPoints(p1, p2);
    _sides[SIDE_BC].setPoints(p2, p3);
    _sides[SIDE_CA].setPoints(p3, p1);
    
    // Calculamos el plano
    _plane.setPoints(pointA, pointB, pointC);
}
             
Cell::~Cell() {}

const Ogre::Vector3& Cell::getCenter() const {
    return _center;
}

const Ogre::Vector3& Cell::getVertex(int index) {
    if (index < 0 || index >= 3) {
        cerr << "Cell::getVertex(): vértice " << index << " inválido" << endl;
        exit(1);
    }
    
    return _vertex[index];
}

Cell* Cell::getLink(CellSide side) const {
    return _links[side];
}

void Cell::setLink(CellSide side, Cell* cell) {
    _links[side] = cell;
}

bool Cell::requestLink(const Ogre::Vector3& pointA,
                       const Ogre::Vector3& pointB,
                       Cell* cell) {
    // Buscamos una pareja de vértices coincidentes
    if (_vertex[VERT_A] == pointA) {
        if (_vertex[VERT_B] == pointB) {
            _links[SIDE_AB] = cell;
            return true;
        }
        else if (_vertex[VERT_C] == pointB) {
            _links[SIDE_CA] = cell;
            return true;
        }
    }
    else if (_vertex[VERT_B] == pointA) {
        if (_vertex[VERT_A] == pointB) {
            _links[SIDE_AB] = cell;
            return true;
        }
        else if (_vertex[VERT_C] == pointB) {
            _links[SIDE_BC] = cell;
            return true;        
        }
    }
    else if (_vertex[VERT_C] == pointA) {
        if (_vertex[VERT_A] == pointB) {
            _links[SIDE_CA] = cell;
            return true;
        }
        else if (_vertex[VERT_B] == pointB) {
            _links[SIDE_BC] = cell;
            return true;
        }
    }
    
    // No hemos encontrado enlace
    return false;
}

static bool sameSide(const Ogre::Vector3& p1,
                     const Ogre::Vector3& p2,
                     const Ogre::Vector3& a,
                     const Ogre::Vector3& b) {
    
    Ogre::Vector3 cp1 = (b - a).crossProduct(p1 - a);
    Ogre::Vector3 cp2 = (b - a).crossProduct(p2 - a);
    
    return (cp1.dotProduct(cp2) >= 0);
}

bool Cell::containsPoint(const Ogre::Vector3& point) const {
    return (sameSide(point, _vertex[VERT_A], _vertex[VERT_B], _vertex[VERT_C]) &&
            sameSide(point, _vertex[VERT_B], _vertex[VERT_C], _vertex[VERT_A]) &&
            sameSide(point, _vertex[VERT_C], _vertex[VERT_A], _vertex[VERT_B]));
            // Podría ser distinto
}

void Cell::getHeight(Ogre::Vector3 &point) {
    point.y = _plane.getY(point.x, point.z);
}

Cell::PathResult Cell::classifyPathToCell(const Line2D& path,
                                          Cell** nextCell,
                                          CellSide &side,
                                          Ogre::Vector2 &intersection) {
    int interiorCount = 0;
    
    for (int i = 0; i < 3; ++i) {
        if (_sides[i].classifyPoint(path.getEndPoint()) != Line2D::RIGHT_SIDE) {
            if (_sides[i].classifyPoint(path.getBeginPoint()) != Line2D::LEFT_SIDE) {
                Line2D::LineClassification result = path.getIntersection(_sides[i], &intersection);
                
                if (result == Line2D::SEGMENTS_INTERSECT || result == Line2D::A_BISECTS_B) {
                    *nextCell = _links[i];
                    side = (CellSide)i;
                    return EXITING_CELL;
                }
            }
        }
        else 
            ++interiorCount;
    }
    
    if (interiorCount == 3) 
        return ENDING_CELL;

    
    return NO_RELATIONSHIP;
}
