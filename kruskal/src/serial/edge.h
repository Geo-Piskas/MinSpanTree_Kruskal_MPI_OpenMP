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

#ifndef EDGE
#define EDGE

// Edge type.
typedef struct edgeType {
    uint32_t u;
    uint32_t v;
    uint32_t weight;
} edge;

// Edge comparison.
int compareEdges(const void *a, const void *b) {
    const edge *pa = (const edge *)a;
    const edge *pb = (const edge *)b;
	if (pa->weight > pb->weight)return 1;
	else if (pa->weight < pb->weight) return -1;
	return 0;
}

#endif
