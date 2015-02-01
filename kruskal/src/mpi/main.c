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
#include <mpi.h>
#include <inttypes.h>
#include "edge.h"
#include "disjointset.h"
#include "qsort_omp.h"

#define VERBOSE 0

FILE *input = NULL;

// MPI related variables.
MPI_Datatype mpiEdge; // MPI datatype of edge.
int mpiNp; // Total # of processors.
int mpiRank; // Rank of a processor.

uint32_t nVerts; // Total # of vertices.
uint32_t nEdges; // Total # of edges.

edge *edges; // Contains the edges of a processor.
edge *edges_sort_buffer; // Used for parallel quicksorting of 'edges' for performance.
uint32_t edgeCount;

edge *mst; // Contains the MST edges of a processor. Also used as send/recv MPI buffer.
uint32_t mstEdgeCount;
uint32_t mstLength;

// Timing variables.
double commTime;
double procTime;
double parseTime;


// Checks if (l  <=  x  <  r)
inline int in(uint32_t x, uint32_t l, uint32_t r) {
    return ((x >= l) && (x < r));
}

// Finalizes MPI and frees buffers.
inline void finalize() {
    MPI_Type_free(&mpiEdge);
    MPI_Finalize();
    free(edges);
    free(mst);
    free(dsSet);
}

// Force kill due to initialization error.
inline void die(char *msg) {
	printf("%d: %s\n",mpiRank, msg);
    if (input != NULL) fclose(input);
    MPI_Type_free(&mpiEdge);
    MPI_Finalize();
    exit(0);
}

// Initializes MPI and mpiEdge datatype.
inline void initMPI(int argc, char **argv) {
	MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpiNp);
    MPI_Type_contiguous(3, MPI_UINT32_T, &mpiEdge);
    MPI_Type_commit(&mpiEdge);
}

// Initializes and validates input.
inline void initInput(char *filename) {
    if ((mpiNp & (mpiNp - 1)) != 0)
        die("* Please make sure that the number of processors is a power of 2\n");

    if(filename == NULL || NULL == (input = fopen(filename, "rb")))
        die("* Provide a valid input file argument.\n");

	fread(&nVerts, sizeof(uint32_t), 1, input);
	fread(&nEdges, sizeof(uint32_t), 1, input);
    if (nVerts/mpiNp < 2)
        die("* The number of vertices per process must be at least 2.\n");
}

// Parses the edges that correspond to the current processor.
inline void parseEdges() {

    uint32_t vPerProc = nVerts/mpiNp; // Vertices per processor.
    uint32_t firstVert = mpiRank * vPerProc; // First vertex index.
    uint32_t lastVert = (mpiRank + 1) * vPerProc; // Last vertex index.

    // Last processor case.
    if (mpiRank == mpiNp - 1) {
        lastVert += nVerts%mpiNp;
        vPerProc += nVerts%mpiNp;
    }

    // Mem allocation.
    edges = malloc(vPerProc * nVerts * sizeof(edge));
    edges_sort_buffer = malloc(vPerProc * nVerts * sizeof(edge));
    
    // Allocating x2 because of merging. The first part of mst buffer contains the
    // MST edges and the second part is used as a receive MPI buffer.
    mst = malloc(2 * (nVerts - 1) * sizeof(edge));

    // Edge parsing.
    edgeCount = 0;
    uint32_t i;
	edge e;
    for (i = 0; i < nEdges; i++) {
		fread(&e, sizeof(edge), 1, input);
        if (in(e.u, firstVert, lastVert) || in(e.v, firstVert, lastVert)) {
            edges[edgeCount++] = e;
        }
    }
    fclose(input);
	parseTime = MPI_Wtime() - parseTime;
    // Waiting for all processors to complete parsing.
	MPI_Barrier(MPI_COMM_WORLD);
}

// Calculates the MST.
inline void calculateMst() {
    // Parallel quicksort.
    qsort_omp(edges, edges_sort_buffer, edgeCount);

    free(dsSet);
    dsMakeSet(nVerts);
    
    // Looping trhough all edges in increasing weight order.
    mstEdgeCount = 0;
    mstLength = 0;
    uint32_t i = 0;
    for(; i < edgeCount; i++) {
        // Find parent sets of the nodes.
        dsNode *vParent = dsFind(&dsSet[edges[i].v]);
        dsNode *uParent = dsFind(&dsSet[edges[i].u]);
        // If they are from two different sets, merge them.
        if(vParent != uParent) {
            mst[mstEdgeCount++] = edges[i];
            mstLength += edges[i].weight;
            dsUnion(vParent, uParent);
        }
    }
}

int main(int argc, char **argv) {
    // Initialization and input file parsing.
    initMPI(argc, argv);
	parseTime = MPI_Wtime();
    initInput(argv[1]);
    parseEdges();

    // Processing.
	double tmpTime;
	procTime = MPI_Wtime();
    
    // In case of 1 processor, act as serial application.
    if (mpiNp == 1) {
        calculateMst();
    } else {
        int processors = mpiNp;
        int pow2 = 1; // Used to find out where to send/recv.
        MPI_Status mpiStatus;
        int recvEdgeCount;
        while(processors > 1) {
            // Calculate the local MST using 'edges' buffer.
            calculateMst();

            // Communication part.
            if((mpiRank/pow2)%2 != 0) {
                // Send from the first half of 'mst'.
                MPI_Send(mst, mstEdgeCount, mpiEdge, (mpiRank-pow2), 0, MPI_COMM_WORLD);
                break; // Processor did his job and can now exit.
            } else {
                // Receive into the second half of 'mst'.
				tmpTime = MPI_Wtime();
                MPI_Recv(mst+mstEdgeCount, nVerts-1, mpiEdge, (mpiRank+pow2), 0, MPI_COMM_WORLD, &mpiStatus);
                MPI_Get_count(&mpiStatus, mpiEdge, &recvEdgeCount);
				commTime += MPI_Wtime() - tmpTime; // Communication time is equal to the waiting-to-recv time.
            }
            // Pointer swap between 'edges' and 'mst'.
            // 'edges' will be reused to calculate a new local MST.
            // 'mst' will contain that new local MST.
            edge *tmp = edges;
            edges = mst;
            mst = tmp;
            edgeCount = mstEdgeCount + recvEdgeCount; // New edge count after merging.

            processors /= 2;
            pow2 *= 2;
        }

		// Only the root will execute this last calculation.
		if(mpiRank == 0) calculateMst();
    }
	procTime = (MPI_Wtime() - procTime) - commTime;


    if(VERBOSE)
	    printf("%d: Parse time: %.3fs\n%d: Comm time: %.3fs\n%d: Proc time: %.3fs\n"
		    , mpiRank, parseTime, mpiRank, commTime, mpiRank, procTime);
	if(mpiRank == 0) {
		printf("MST length: %d\n", mstLength);
		printf("Total Time %.3fs\n", parseTime + commTime + procTime);
		printf("Without I/O %.3fs\n", commTime + procTime);
	}
    finalize();
    return 0;
}
