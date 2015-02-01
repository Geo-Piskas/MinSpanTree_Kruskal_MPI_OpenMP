/*
 * Kruskal's MPI
 * Copyright (C) 2015 George Piskas
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact: geopiskas@gmail.com
 */

#ifndef DISJOINT
#define DISJOINT

#include <stdlib.h>

// Disjoint Set (ds) type.
typedef struct dsNodeType {
    struct dsNodeType* parent;
    uint32_t rank;
} dsNode;

// Set that contains all vertices as dsNodes.
dsNode *dsSet;

// MakeSet operation.
inline void dsMakeSet(uint32_t nVerts) {
    // Callocates (init to 0) room for all vertices.
    // parent = NULL and rank = 0.
	dsSet = calloc(nVerts, sizeof(dsNode));
}

// Find operation.
inline dsNode* dsFind(dsNode *n) {
    if (n->parent == NULL) return n;
    n->parent = dsFind(n->parent);
    return n->parent;
}

// Union operation.
// Assuming that n and m belong to different sets.
inline void dsUnion(dsNode* n, dsNode* m) {
    if (n->rank < m->rank) {
        n->parent = m;
    } else if (n->rank > m->rank) {
        m->parent = n;
    } else {
        n->parent = m;
        n->rank += 1;
    }
}

#endif
