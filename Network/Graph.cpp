#include "stdafx.h"
#include "Graph.h"

using namespace std;

Graph::Graph(){}

Graph::Graph(Network* network) {
	nb_nodes = network->getSumofNodes();

	// read cumulative degree sequence: 4 bytes for each node
	// cum_degree[0]=degree(0); cum_degree[1]=degree(0)+degree(1), etc.

	degrees = (int *)malloc((long)nb_nodes * sizeof(int));
	degrees[0] = network->nb_Neighbors(network->get_Node(0, INDEX));
	for (int i = 1; i < nb_nodes; i++){
		degrees[i] = degrees[i - 1] + network->nb_Neighbors(network->get_Node(i, INDEX));
	}

	// read links: 4 bytes for each link (each link is counted twice)
	nb_links = network->nb_Undirected_Links();
	links = (int *)malloc((long)nb_links * sizeof(int) * 2);
	
	network->undirected_Links(links);
	

	//read weights: 4 bytes for each link (each link is counted twice)
	weights = (int *)malloc((long)nb_links * sizeof(int) * 2);
	
	total_weight = 2 * network->cal_Total_Weight(); // total_weight 无向边总权重的两倍
	int idx = 0;
	for (int i = 0; i < nb_nodes; i++){
		for (int j = 0; j < nb_nodes; j++){
			if (i == j) continue;
			int weight = network->undirected_Weight(network->get_Node(i, INDEX), network->get_Node(j, INDEX));
			if (weight != 0){
				weights[idx++] = weight;
			}
		}
	}

	cout<<"nodes" << nb_nodes << endl;
	cout<<"links" << nb_links << endl;
	cout << "total_weight" << total_weight << endl;
	
}

Graph::~Graph(){
	free(degrees);
	free(links);
	free(weights);
}

int
Graph::nb_neighbors(int node) {
	assert(node >= 0 && node<nb_nodes);

	if (node == 0)
		return degrees[0];
	else
		return degrees[node] - degrees[node - 1];
}

int
Graph::nb_selfloops(int node) {
	assert(node >= 0 && node<nb_nodes);

	pair<int *, int *> p = neighbors(node);
	for (int i = 0; i<nb_neighbors(node); i++) {
		if (*(p.first + i) == node) {
			if (weights != NULL)
				return *(p.second + i);
			else
				return 1;
		}
	}
	return 0;
}

 int
Graph::weighted_degree(int node) {
	assert(node >= 0 && node<nb_nodes);

	pair<int *, int *> p = neighbors(node);
	if (p.second == NULL)
		return nb_neighbors(node);
	else {
		int res = 0;
		for (int i = 0; i<nb_neighbors(node); i++)
			res += *(p.second + i);
		return res;
	}
}

inline pair<int *, int *>
Graph::neighbors(int node) {
	assert(node >= 0 && node<nb_nodes);

	if (node == 0)
		return make_pair(links, weights);
	else if (weights != NULL)
		return make_pair(links + (long)degrees[node - 1], weights + (long)degrees[node - 1]);
	else
		return make_pair(links + (long)degrees[node - 1], weights);
}

void
Graph::display() {
	for (int node = 0; node<nb_nodes; node++) {
		pair<int *, int *> p = neighbors(node);
		for (int i = 0; i<nb_neighbors(node); i++) {
			if (weights != NULL)
				cout << node << " " << *(p.first + i) << " " << *(p.second + i) << endl;
			else {
				cout << (node + 1) << " " << (*(p.first + i) + 1) << endl;
				//	cout << (node) << " " << (*(p.first+i)) << endl;
			}
		}
	}
}

void
Graph::display_binary(char *outfile) {
	ofstream foutput;
	foutput.open(outfile, fstream::out | fstream::binary);

	foutput.write((char *)(&nb_nodes), 4);
	foutput.write((char *)(degrees), 4 * nb_nodes);
	foutput.write((char *)(links), 8 * nb_links);
}