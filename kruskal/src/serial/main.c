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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>
#include "edge.h"
#include "disjointset.h"

#define VERBOSE 0

// Utility function to convert ASCII input files to binary.
void toBinary(char **argv) {
    FILE *input = NULL;
    FILE *out = fopen(argv[2], "wb");
	if(argv[1] == NULL || NULL == (input = fopen(argv[1], "r"))) {
        printf("* Provide a valid input file argument\n");
        exit(0);
    }

    uint32_t nVerts, nEdges, i;
    fscanf(input,"%d %d",&nVerts, &nEdges);
	fwrite(&nVerts, sizeof(uint32_t), 1, out);
	fwrite(&nEdges, sizeof(uint32_t), 1, out);

	edge e;
    for (i = 0; i < nEdges; i++) {
		fscanf(input, "%d %d %d", &e.u, &e.v, &e.weight);
		fwrite(&e, sizeof(edge), 1, out);
    }
	fclose(out);
	fclose(input);
    printf("* Done\n");
}


int main(int argc, char **argv) {
    // If an extra argument is given, launch the conversion utility.
	if (argc == 3) {
		toBinary(argv);
		exit(0);
	}

    // Input parsing and timing.
	double parseTime = clock();
    FILE *input = NULL;
	if(argv[1] == NULL || NULL == (input = fopen(argv[1], "rb"))) {
        printf("* Provide a valid input file argument\n");
        exit(0);
    }

    uint32_t nVerts, nEdges, i;
	fread(&nVerts, sizeof(uint32_t), 1, input); // Total # of vertices.
	fread(&nEdges, sizeof(uint32_t), 1, input); // Total # of edges.
    edge *edges = malloc(nEdges * sizeof(edge)); // Contains all edges.
    edge *mst =  malloc((nVerts-1) * sizeof(edge)); // Contains the MST edges.

    // Reading edges from the file into 'edges' array.
    for (i = 0; i < nEdges; i++)
		fread(&edges[i], sizeof(edge), 1, input);
	
	parseTime = clock() - parseTime;
    // End of input parsing.
    
    // MST calculation.
	double procTime = clock();
    // Edge sorting using quicksort.
    qsort(edges, nEdges, sizeof(edge), compareEdges);
    dsMakeSet(nVerts);

    // Looping trhough all edges in increasing weight order.
    uint32_t mstLength = 0;
    uint32_t nMstEdges = 0;
    for(i = 0; i < nEdges; i++) {
        // Find parent sets of the nodes.
        dsNode *vParent = dsFind(&dsSet[edges[i].v]);
        dsNode *uParent = dsFind(&dsSet[edges[i].u]);
        // If they are from two different sets, merge them.
        if(vParent != uParent) {
            mst[nMstEdges++] = edges[i];
            mstLength += edges[i].weight;
            dsUnion(vParent, uParent);
        }
    }
	procTime = clock() - procTime;
    // End of processing.

    if (VERBOSE) {
	    printf("Parse time: %.3fs\n", parseTime/CLOCKS_PER_SEC);
	    printf("Proc time: %.3fs\n", procTime/CLOCKS_PER_SEC);
    }

	printf("MST length: %d\n", mstLength);
	printf("Total Time %.3fs\n", (parseTime + procTime)/CLOCKS_PER_SEC);
	printf("Without I/O %.3fs\n", procTime/CLOCKS_PER_SEC);

	fclose(input);
    free(edges);
    free(mst);
    free(dsSet);
    return 0;
}
