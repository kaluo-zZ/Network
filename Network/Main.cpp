/*
// 调试内存泄漏时需要
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
*/


#include"stdafx.h"
#include <stack>
#include "Network.h"
#include "Graph.h"
#include "Community.h"
#include <thread>
#include <time.h>


using namespace std;

typedef struct Demand{
	int DemandID;
	int SourceID;
	int DestinationID;
	double Bandwidth;

}Demand;

double precision = 0.000001;
ofstream outfile;

//模板函数：将string类型变量转换为常用的数值类型（此方法具有普遍适用性）  
template <class Type>
Type stringToNum(const string& str)
{
	istringstream iss(str);
	Type num;
	iss >> num;
	return num;
}

//字符串分割函数  
vector<string> split(string str, string pattern)
{
	string::size_type pos;
	vector<std::string> result;
	str += pattern;//扩展字符串以方便操作  
	int size = str.size();

	for (int i = 0; i<size; i++)
	{
		pos = str.find(pattern, i);
		if (pos<size)
		{
			std::string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}


void print(Network* graph, int s, int d, double bandwidth){
	Node* no = graph->get_Node(s, ID);
	Node* des = graph->get_Node(d, ID);
	//cout << "起点" << s << "，终点" << d << "带宽" << bandwidth << ",路径为：";
	Edge* e = NULL;
	if (no->pre == NULL){
		//cout << "NULL" << endl;
		outfile << "NULL" << endl;
		return;
	}
	while (no->ID != des->ID){
		//cout << no->ID << "-->";
		outfile << no->ID << ",";
		e = graph->get_Edge(no->ID, no->pre->ID);
		e->availableBandwidth = e->availableBandwidth - bandwidth;
		no = no->pre;
	}
	//cout << des->ID << endl;
	outfile << des->ID << endl;
}

void modify_comm_idx(vector<Community*> comms,Network* network, vector<int> v, int level, int comm_idx){
	if (level < -1) return;
	if (level == -1){
		vector<int>::iterator it = v.begin();
		for (; it != v.end(); it++){
			network->get_Node(*it, INDEX)->community = comm_idx;
		}
		return;
	}
	
	Community* comm = comms[level];
	vector<int>::iterator it = v.begin();
	for (; it != v.end(); it++){
		modify_comm_idx(comms, network, comm->comm_nodes[*it], level - 1, comm_idx);
	}
}

void run_demands(Network* network, vector<Demand*> demand, vector<Demand*> outter_demand){
	for (int i = 0; i < demand.size(); i++){
		network->CSPF(demand[i]->SourceID, demand[i]->DestinationID, demand[i]->Bandwidth);
		Node* no = network->get_Node(demand[i]->SourceID, ID);
		if (no->pre == NULL){
			outter_demand.push_back(demand[i]);
		}
		else
			print(network, demand[i]->SourceID, demand[i]->DestinationID, demand[i]->Bandwidth);
	}
}


int partition(Demand** pointer_to_demand, int p, int r){
	Demand* x = pointer_to_demand[r];
	int i = p - 1;
	for (int j = p; j < r; j++){
		if (pointer_to_demand[j]->Bandwidth >= x->Bandwidth){
			i += 1;
			Demand* d = pointer_to_demand[i];
			pointer_to_demand[i] = pointer_to_demand[j];
			pointer_to_demand[j] = d;
		}
	}
	Demand* d = pointer_to_demand[i + 1];
	pointer_to_demand[i + 1] = pointer_to_demand[r];
	pointer_to_demand[r] = d;
	return i + 1;
}
void  quickSort(Demand** pointer_to_demand, int p, int r){
	if (p < r){
		int q = partition(pointer_to_demand, p, r);
		quickSort(pointer_to_demand, p, q - 1);
		quickSort(pointer_to_demand, q + 1, r);
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	//_CrtSetBreakAlloc(3575816);
	ifstream file("dc_topo_new7.csv");
	ifstream demand_file("dc_demand7.csv");
	outfile.open("dc_demand7_path.txt");
	string value;
	vector<string> vec;
	Network* network = new Network();
	Edge* edge;
	vector<Community*> communities;

	// 导入网络拓扑
	getline(file, value); //去掉表头
	while (file.good()){
		getline(file, value);
		vec = split(value, ",");
		if (vec.size() != 10)
			continue;

		edge = new Edge();
		edge->LinkID = stringToNum<int>(vec[0]);
		edge->Cost = stringToNum<double>(vec[4]);
		edge->Bandwidth = stringToNum<double>(vec[5]);

		network->add_Edge(edge, stringToNum<int>(vec[1]), stringToNum<int>(vec[2]));
	}

	// 导入业务集
	getline(demand_file, value);
	Demand* demands = (Demand*)malloc(20000 * sizeof(Demand));
	for (int i = 0; i < 10000; i++){
		getline(demand_file, value);

		vec = split(value, ",");
		int id = stringToNum<int>(vec[0]);
		int s = stringToNum<int>(vec[1]);
		int d = stringToNum<int>(vec[2]);
		double bandwidth = stringToNum<double>(vec[5]);

		demands[i].DemandID = id;
		demands[i].SourceID = s;
		demands[i].DestinationID = d;
		demands[i].Bandwidth = bandwidth;
	}

	
	// 进行网络社团划分

	Graph*graph = new Graph(network);

	Community* c = new Community(graph, -1, precision);
	double mod = c->modularity();
	std::cout << mod << endl;
	std::cerr << "network : "
		<< c->g->nb_nodes << " nodes, "
		<< c->g->nb_links << " links, "
		<< c->g->total_weight << " weight." << endl;
	
	double new_mod = c->one_level();

	std::cerr << "modularity increased from " << mod << " to " << new_mod << endl;
	
	Graph* g = c->partition2graph_binary();

	std::cerr << "network of communities computed" << endl;
	communities.push_back(c);

	while (new_mod - mod>precision) {
		
		mod = new_mod;
		Community* c = new Community(g, -1, precision);

		std::cerr << "\nnetwork : "
			<< c->g->nb_nodes << " nodes, "
			<< c->g->nb_links << " links, "
			<< c->g->total_weight << " weight." << endl;

		new_mod = c->one_level();

		std::cerr << "communities computed" << endl;
		std::cerr << "modularity increased from " << mod << " to " << new_mod << endl;

		Graph* g1 = c->partition2graph_binary();
		g = g1;

		std::cerr << "network of communities computed" << endl;
		
		communities.push_back(c);

	}


	int len = communities.size();
	int size = communities[len - 1]->comm_nodes.size();
	for (int i = 0; i < size;i++){
		vector<int> v = communities[len - 1]->comm_nodes[i];
		modify_comm_idx(communities, network, v, len - 2, i);
	}

	
	vector<Network*> networks(size);  // 子图
	vector<vector<Demand*>> classified_demands(size + 1); // 子图对应的域内业务，最后一个存储域间业务
	// 输出划分的社团及其包含节点数目
	vector<int> count(size, 0);
	for (int i = 0; i < network->getSumofNodes(); i++){
		count[network->get_Node(i, INDEX)->community] ++;
	}
	int total = 0;
	for (int i = 0; i < size; i++){
		total += count[i];
		cout << i << ":" << count[i] << endl;
	}
	cout <<"total:"<< total << endl;
	
	for (int i = 0; i < size; i++){
		networks[i] = new Network();
	}

	

	// 为子图分配边，当边的起点和终点在同一社团时，加入这个子图
	for (int i = 0; i < network->getSumofNodes(); i++){
		Node* n = network->get_Node(i, INDEX);
		Edge* e = n->out;
		while (e != NULL){
			if (e->Source->community == e->Destination->community){
				Edge* edge = new Edge();
				edge->LinkID = e->LinkID;
				edge->Cost = e->Cost;
				edge->Bandwidth = e->Bandwidth;
				networks[e->Source->community]->add_Edge(edge, e->Source->ID, e->Destination->ID);
			}
			e = e->right;
		}
	}

	clock_t start = clock();
	//为每个子图分配业务
	for (int i = 0; i < 10000; i++){
		int c1 = network->get_Node(demands[i].SourceID, ID)->community;
		int c2 = network->get_Node(demands[i].DestinationID, ID)->community;
		if (c1 == c2){ // 域内业务
			classified_demands[c1].push_back(demands + i);
		}
		else{ // 域间业务
			classified_demands[size].push_back(demands + i);
		}
	}
	cout << endl;
	total = 0;
	for (int i = 0; i < size + 1; i++){
		total += classified_demands[i].size();
		cout << i << ":" << classified_demands[i].size() << endl;
	}
	cout << "total:" << total << endl;

	/*
	vector<Demand**> pointer_to_demand(size + 1);
	for (int i = 0; i < size; i++){
		pointer_to_demand[i] = new Demand*[classified_demands[i].size()];
		for (int j = 0; j < classified_demands[i].size(); j++){
			pointer_to_demand[i][j] = classified_demands[i][j];
		}
		quickSort(pointer_to_demand[i], 0, classified_demands[i].size() - 1);
	}
	*/
	

	cout << "计算路由中..." << endl;
	
	for (int i = 0; i < size; i++){
		thread t(run_demands, networks[i], classified_demands[i], classified_demands[size]);
		//thread t(run_demands, networks[i], pointer_to_demand[i], classified_demands[size]);
		t.join();
	}
	
	
	// 将子网络的带宽使用情况反映在原网络中
	for (int i = 0; i < size; i++){
		for (int j = 0; j < networks[i]->getSumofNodes(); j++){
			Node* n = networks[i]->get_Node(j, INDEX);
			Edge* e = n->out;
			while (e != NULL){
				Edge* edge = network->get_Edge(e->Source->ID, e->Destination->ID);
				edge->availableBandwidth = e->availableBandwidth;
				e = e->right;
			}
		}
	}
	
	// 计算域间业务及对上一次求域内路由不成功的业务进行重路由
	vector<Demand*> ds = classified_demands[size];
	for (int i = 0; i < ds.size(); i++){
		network->CSPF(ds[i]->SourceID, ds[i]->DestinationID, ds[i]->Bandwidth);
		print(network, ds[i]->SourceID, ds[i]->DestinationID, ds[i]->Bandwidth);
	}
	
	clock_t end = clock();
	cout << "时间：" << static_cast<double>(end - start) / CLOCKS_PER_SEC * 1000 << "ms" << endl;
	cout << "路由计算结束" << endl;
	
	/*
	ofstream ofs("2000.csv");
	ofs <<"DemandID"<<","<< "SourceID"<<"," << "DestinationID"<<"," << "Bandwidth" << endl;
	for (int i = 0; i < ds.size(); i++){
		ofs << ds[i]->DemandID << "," << ds[i]->SourceID << "," << ds[i]->DestinationID << "," << ds[i]->Bandwidth << endl;
	}
	ofs.close();
	
	// 输出节点及其所属的社团号
	ofstream ofs("0.csv");
	ofs << "Id" << "," << "Community" << endl;
	for (int i = 0; i < network->getSumofNodes(); i++){
		Node* n = network->get_Node(i, INDEX);
		ofs << (double)(n->ID) << "," << n->community << endl;
	}
	ofs.close();
	*/




	// 释放内存
	for (int i = 0; i < len; i++)
		delete communities[i];
	for (int i = 0; i < size; i++)
		delete networks[i];
	delete network;
	free(demands);

	file.close();
	demand_file.close();
	outfile.close();
	//_CrtDumpMemoryLeaks();
	
	return 0;
}