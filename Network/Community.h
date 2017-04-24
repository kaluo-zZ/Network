#ifndef COMMUNITY_H
#define COMMUNITY_H

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <map>

#include "Graph.h"

using namespace std;

class Community {
public:
	// network to compute communities for
	Graph* g;

	// number of nodes in the network and size of all vectors
	int size;

	vector<vector<int> > comm_nodes;

	// community to which each node belongs
	vector<int> n2c;

	// used to compute the modularity participation of each community
	vector<int> in, tot;

	// number of pass for one level computation
	// if -1, compute as many pass as needed to increase modularity
	int nb_pass;

	// a new pass is computed if the last one has generated an increase 
	// greater than min_modularity
	// if 0. even a minor increase is enough to go for one more pass
	double min_modularity;


	// constructors:
	// reads graph from file using graph constructor
	// copy graph
	Community(Graph* g, int nb_pass, double min_modularity);

	~Community(){
		delete g;
		g = NULL;
	}


	// display the community of each node
	void display();

	// remove the node from its current community with which it has dnodecomm links
	inline void remove(int node, int comm, int dnodecomm);

	// insert the node in comm with which it shares dnodecomm links
	inline void insert(int node, int comm, int dnodecomm);

	// compute the gain of modularity if node where inserted in comm
	// given that node has dnodecomm links to comm.  The formula is:
	// [(In(comm)+2d(node,comm))/2m - ((tot(comm)+deg(node))/2m)^2]-
	// [In(comm)/2m - (tot(comm)/2m)^2 - (deg(node)/2m)^2]
	// where In(comm)    = number of half-links strictly inside comm
	//       Tot(comm)   = number of half-links inside or outside comm (sum(degrees))
	//       d(node,com) = number of links from node to comm
	//       deg(node)   = node degree
	//       m           = number of links
	inline double modularity_gain(int node, int comm, int dnodecomm);

	// compute the set of neighboring communities of node
	// for each community, gives the number of links from node to comm
	map<int, int> neigh_comm(int node);

	// compute the modularity of the current partition
	double modularity();

	// displays the graph of communities as computed by one_level
	void partition2graph();
	// displays the current partition (with communities renumbered from 0 to k-1)
	void display_partition();

	// generates the binary graph of communities as computed by one_level
	Graph* partition2graph_binary();

	// compute communities of the graph for one level
	// return the modularity
	double one_level();
};
     
#endif // COMMUNITY_H
