#ifndef GRAPH_H
#define GRAPH_H

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include "Network.h"

using namespace std;

class Graph {
public:
	int nb_nodes;
	int nb_links;
	int total_weight;

	int *degrees;
	int *links;
	int *weights;

	Graph();

	// binary file format is
	// 4 bytes for the number of nodes in the graph
	// 4*(nb_nodes) bytes for the cumulative degree for each node:
	//    deg(0)=degrees[0]
	//    deg(k)=degrees[k]-degrees[k-1]
	// 4*(sum_degrees) bytes for the links
	// IF WEIGHTED 4*(sum_degrees) bytes for the weights
	Graph(Network* network);
	~Graph();

	void display(void);
	void display_binary(char *outfile);

	// return the number of neighbors (degree) of the node
	 int nb_neighbors(int node);

	// return the number of self loops of the node
	 int nb_selfloops(int node);

	// return the weighted degree of the node
	 int weighted_degree(int node);

	// return pointers to the first neighbor and first weight of the node
	inline pair<int *, int *> neighbors(int node);
};




#endif // GRAPH_H
