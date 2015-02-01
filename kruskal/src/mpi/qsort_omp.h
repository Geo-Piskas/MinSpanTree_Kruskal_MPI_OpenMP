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

#ifndef QSORT_OMP
#define QSORT_OMP

#include <stdlib.h>
#include <omp.h>
#include "edge.h"

// Number of threads participating in the sorting.
#define NUM_THREADS 2

void qsort_omp(edge* edges, edge* buffer, size_t size) {
    int N = NUM_THREADS;
	omp_set_num_threads(N);
    
    // "Split" input array into N parts.
	int i;
	int *index = malloc((N+1)*sizeof(int));
	for(i = 0; i < N; i++)
		index[i] = (i*size)/N;
	index[N] = size;

    // Each thread will qsort a part.
    #pragma omp parallel for private(i)
	for(i = 0; i < N; i++)
		qsort(edges+index[i], index[i+1]-index[i], sizeof(edge), compareEdges);

    // Finally, a parallel mergesort merges the sorted parts.
	do {
        #pragma omp parallel for private(i) 
	    for(i = 0; i < N; i += 2) {
            edge* A = edges+index[i];
            edge* B = edges+index[i+1];
            edge* bfr = buffer+index[i];
            int aSize = index[i+1]-index[i];
            int bSize = index[i+2]-index[i+1];
        	int ai=0, bi=0, j=0;
            while (ai < aSize && bi < bSize) {
	            if (A[ai].weight <= B[bi].weight)
		            bfr[j++] = A[ai++];
	            else
		            bfr[j++] = B[bi++];
            }
            int k;
            if (ai < aSize) 
                for (k = ai; k < aSize; k++) bfr[j++] = A[k];
            else
                for (k = bi; k < bSize; k++) bfr[j++] = B[k];
        }
	    N /= 2;
	    for(i = 0; i < N; i++)
            index[i] = (i*size)/N;
        index[N] = size;
	} while(N > 1);
    edge *tmp = edges;
    edges = buffer;
    buffer = tmp;
}

#endif
