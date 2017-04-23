// Network.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include <stack>
#include "Network.h"

/*采用十字链表的图结构*/
using namespace std;

Node* Network::add_Node(int ID){
	/*
	根据ID添加节点，如果节点已经添加过，这直接返回指向该节点的指针;
	修改IDToPointer映射表。
	*/
	hash_map<int, Node*>::iterator iter = IDToPointer.find(ID);
	if (iter != IDToPointer.end()){ // 如果ID已存在，则直接返回指向该节点的指针
		return iter->second;
	}
	if (Sum_of_Nodes >= Size){ //若空间超出已分配空间，重新分配内存空间
		if ((nodes = (Node*)realloc(nodes, Size + 500)) == NULL){
			return false;
		}
		Size += 500;
		int i = 0;
		for (iter = IDToPointer.begin(); iter != IDToPointer.end(); ++iter){
			iter->second = nodes + i; // 修改所有节点的指针
		}
	}

	nodes[Sum_of_Nodes].ID = ID;
	nodes[Sum_of_Nodes].community = 0;
	nodes[Sum_of_Nodes].out = NULL;
	nodes[Sum_of_Nodes].in = NULL;
	nodes[Sum_of_Nodes].d = 0;
	nodes[Sum_of_Nodes].pres = NULL;
	nodes[Sum_of_Nodes].pre = NULL;
	nodes[Sum_of_Nodes].flag = false;
	nodes[Sum_of_Nodes].hops = INT_MAX;
	nodes[Sum_of_Nodes].pointer_index = 0;

	IDToPointer.insert(hash_map<int, Node*>::value_type(ID, nodes + Sum_of_Nodes)); // 记录ID和指针对应关系

	Sum_of_Nodes++;
	return nodes + Sum_of_Nodes - 1;
}
bool Network::add_Edge(Edge* edge, int SourceID, int DestinationID){
	if (edge == NULL){
		return false;
	}
	Node* source = add_Node(SourceID);
	Node* destination = add_Node(DestinationID);

	edge->down = NULL;
	edge->right = NULL;
	edge->Source = source;
	edge->Destination = destination;
	edge->availableBandwidth = edge->Bandwidth;

	/*添加边，并调整指针*/
	if (source->out == NULL){
		source->out = edge;
	}
	else{
		Edge* e = source->out;
		while (e->right != NULL){
			e = e->right;
		}
		e->right = edge;
	}
	if (destination->in == NULL){
		destination->in = edge;
	}
	else{
		Edge* e = destination->in;
		while (e->down != NULL){
			e = e->down;
		}
		e->down = edge;
	}
	/*边总数加一*/
	Sum_of_Edges++;
	total_weight++;
	graph_Change = true;
	return true;
}
Node* Network::get_Node(int value, Type type = ID){
	if (type == ID){
		hash_map<int, Node*>::iterator iter = IDToPointer.find(value);
		if (iter == IDToPointer.end())
			return NULL;
		return iter->second;

	}
	else{
		return nodes + value;
	}

}
Edge* Network::get_Edge(int sourceID, int destinationID){
	Node* u = get_Node(sourceID);
	if (u == NULL)
		return NULL;
	Edge* e = u->out;
	while (e != NULL){
		if (e->Destination->ID == destinationID){
			return e;
		}
		e = e->right;
	}
	return NULL;

}
void Network::initialize_Single_Source(Node* s){
	// 对最短路径估计和前驱节点进行初始化
	for (int i = 0; i < Sum_of_Nodes; i++){
		nodes[i].d = INT_MAX;
		nodes[i].pres = NULL;
	}
	s->d = 0;
}
void Network::relax(Edge* e, Priority_Queue* pri_q){
	// 对(u, v)进行松弛
	Node* u = e->Source;
	Node* v = e->Destination;
	double weight = e->Cost;

	Path_Link* s = v->pres;

	if (v->d > u->d + weight){
		pri_q->Decrease_Key(v, u->d + weight);
		// 删除记录前驱节点的块
		while (s != NULL){
			Path_Link* temp = s->next_Link;
			delete s;
			s = temp;
		}
		Path_Link* pl = new Path_Link(); // 新建Path_Link集初始化
		pl->availableBandwidth = e->availableBandwidth;
		pl->next_Link = NULL;
		pl->pre_node = u;
		pl->flag = false;
		v->pres = pl;
	}
	else if (v->d == u->d + weight){
		Path_Link* pl = new Path_Link(); // 新建Path_Link集初始化
		pl->availableBandwidth = e->availableBandwidth;
		pl->next_Link = NULL;
		pl->pre_node = u;
		pl->flag = false;
		if (s == NULL){
			v->pres = pl;
		}
		else{
			while (s->next_Link){
				s = s->next_Link;
			}
			s->next_Link = pl;
		}
	}
}

void Network::CSPF(int sourceID, int destinationID, double bandwidth){
	Node* s = get_Node(sourceID);
	//std::vector<Node> S; // 节点集合S
	Node* des = get_Node(destinationID);

	if (!s || !des) return;

	initialize_Single_Source(s); // 放在创建最小优先队列之前
	Priority_Queue* pri_q = new Priority_Queue(nodes, Sum_of_Nodes); //创建最小优先队列

	
	while (!pri_q->Empty()){
		Node* u = pri_q->Extract_Min();
		//S.push_back(u);
		// 对u的每条出边进行松弛操作
		//cout << u->ID << endl << endl;
		if (u->ID == destinationID) break;
		if (u->d == INT_MAX) {
			//cout << ".";
			break;
		};
		Edge* e = u->out;
		while (e != NULL){
			if (e->availableBandwidth >= bandwidth) // 带宽满足业务要求
				relax(e, pri_q);
			e = e->right;
		}
		
	}

	/*找到子图中从s到des的最大带宽的路径*/
	vector<Path_Link*> pls;
	vector<Path_Link*> pls_v;// 记录子图中满足从s到des的最大带宽的Path_Link型节点的指针
	while (true){
		// 初始化
		for (int i = 0; i < Sum_of_Nodes; i++){
			nodes[i].flag = false;
		}
		/*第一次从des往前回溯，若s->flag为假，即未被访问，则不存在从des到s的路径*/
		s->flag = false;
		pls.clear();
		DFS_Visit(des, s, pls); // 找到并返回子图中具有最小带宽的Path_Link型节点的指针

		if (!s->flag) // 如果s->flag为假，则说明不经过子图中的最小带宽的边无法到达终点，此时这条边所对应的带宽是业务的最大带宽
			break;
		pls_v.clear();
		for (int i = 0; i < pls.size(); i++){// 屏蔽这些边
			pls[i]->flag = true;
			pls_v.push_back(pls[i]);
		}
	}

	if (pls_v.empty()){ // 没有满足带宽要求的路径
		//s->pre = NULL;
		return;
	}
	for (int i = 0; i < pls_v.size(); i++){ // 取消屏蔽(因为最后一次无法到达终点)
		pls_v[i]->flag = false;
	}

	/*找到最小跳数的路径，在这个代码块中会修改节点的pre指针，使其指向具有最小hops的节点*/
	
	for (int i = 0; i < Sum_of_Nodes; i++){
		nodes[i].flag = false;
		nodes[i].hops = INT_MAX;
		nodes[i].pre = NULL;
	}
	des->hops = 0;
	min_Hops(des, s);

	/*后续处理，判断路径是否存在*/
	Node* no = s;
	while (no != des && no->pre != NULL){
		no = no->pre;
	}
	if (no != des){
		s->pre = NULL;
	}
	// 释放Path_Link块
	Path_Link* link = NULL;
	for (int i = 0; i < Sum_of_Nodes; i++){
		link = nodes[i].pres;
		nodes[i].pres = NULL;
		while (link != NULL){
			Path_Link* temp = link->next_Link;
			delete link;
			link = temp;
		}
	}

	// 释放最小优先队列
	delete pri_q;
}

void Network::DFS_Visit(Node* u, Node* s, vector<Path_Link*>& pls){
	/*
	从u开始遍历由CSPF产生的子图，找到并返回子图中带宽最小的边的指针。
	再次调用该函数时会在原来基础上（不经过上一次运行时屏蔽的边）运行。
	*/
	if (u == NULL || s == NULL) return;
	if (u->flag){
		return;
	}

	u->flag = true; // 节点u已访问
	//cout << u->ID << "-";
	if (u->ID == s->ID) {
		//cout << u->ID << " ";
		return;
	}
	Path_Link* pl = u->pres;
	if (pl == NULL) // 没有边
		return;
	Path_Link* pLink = NULL;
	while (pl != NULL){
		if (pl->flag){// 如果这条边被屏蔽
			pl = pl->next_Link;
			continue;
		}
		DFS_Visit(pl->pre_node, s, pls);

		if (pls.empty())
			pls.push_back(pl);
		else{
			if (pls[0]->availableBandwidth > pl->availableBandwidth){
				pls.clear();
				pls.push_back(pl);
			}
			else if (pls[0]->availableBandwidth == pl->availableBandwidth){
				pls.push_back(pl);
			}
		}

		pl = pl->next_Link;
	}
	return;
}

void Network::min_Hops(Node* u, Node* s){
	Path_Link* pl = u->pres;
	if (pl == NULL) return;
	if (u->ID == s->ID) return;

	while (pl != NULL){
		if (pl->flag) {
			pl = pl->next_Link;
			continue;
		}
		if (u->hops + 1 < pl->pre_node->hops){
			pl->pre_node->hops = u->hops + 1;
			pl->pre_node->pre = u;	
			min_Hops(pl->pre_node, s);
		}
		
		pl = pl->next_Link;
	}


}

int Network::nb_Out_Neighbors(Node* node){
	int nb = 0;
	Edge* n = node->out;
	while (n != NULL){
		++nb;
		n = n->right;
	}
	return nb;
}

int Network::nb_In_Neighbors(Node* node){
	int nb = 0;
	Edge* n = node->in;
	while (n != NULL){
		++nb;
		n = n->down;
	}
	return nb;
}

int Network::nb_Neighbors(Node* node){
	// 连接节点node的邻居节点数
	int nb = 0;
	Edge* out = node->out;
	while (out != NULL){
		out->Destination->flag = false;
		out = out->right;
	}
	Edge* in = node->in;
	while (in != NULL){
		in->Source->flag = false;
		in = in->down;
	}
	// 开始计算
	out = node->out;
	while (out != NULL){
		if (!out->Destination->flag){
			nb++;
			out->Destination->flag = true;
		}
		out = out->right;
	}

	in = node->in;
	while (in != NULL){
		if (!in->Source->flag){
			nb++;
			out->Source->flag = true;
		}
		in = in->down;
	}
	return nb;
}

int Network::undirected_Weight(Node* n1, Node* n2){
	int res = 0;
	Edge* e = n1->out;
	while (e != NULL){
		if (e->Destination == n2){
			res += 1;
			break;
		}
		e = e->right;
	}
	e = n1->in;
	while (e != NULL){
		if (e->Source == n2){
			res += 1;
		}
		e = e->down;
	}
	return res;
}

int Network::weighted_Degree(Node* node){
	int res = 0;
	Edge* e = node->out;
	while (e != NULL){
		res += 1;
		e = e->right;
	}
	e = node->in;
	while (e != NULL){
		res += 1;
		e = e->down;
	}
	return res;
}

double Network::cal_Total_Weight(){
	return total_weight;
}

int Network::nb_Undirected_Links(){
	
	int res = 0;
	if (graph_Change){
		for (int i = 0; i < Sum_of_Nodes; i++){
			Edge* e = nodes[i].out;
			while (e != NULL){
				e->flag = false;
				e = e->right;
			}
		}
		for (int i = 0; i < Sum_of_Nodes; i++){
			Edge* e = nodes[i].out;
			while (e != NULL){
				Edge* edge = get_Edge(e->Destination->ID, e->Source->ID);
				if (edge == NULL){
					e->flag = true;
					res++;
					e = e->right;
					continue;
				}
				if (!(edge->flag)){
					e->flag = true;
					res++;
				}
				e = e->right;
			}
		}
			links = res;
			graph_Change = false;
	}
	else{
		res = links;
	}
	return res;
}

void Network::undirected_Links(int* links){
	int sum = 0;
	for (int i = 0; i < Sum_of_Nodes; i++){
		nodes[i].idx = i;
	}
	for (int i = 0; i < Sum_of_Nodes; i++){
		// 初始化
		Edge* out = nodes[i].out;
		while (out != NULL){
			out->Destination->flag = false;
			out = out->right;
		}
		Edge* in = nodes[i].in;
		while (in != NULL){
			in->Source->flag = false;
			in = in->down;
		}

		// 开始计算
		out = nodes[i].out;
		while (out != NULL){
			if (!out->Destination->flag){
				links[sum++] = out->Destination->idx;
				out->Destination->flag = true;
			}
			out = out->right;
		}

		in = nodes[i].in;
		while (in != NULL){
			if (!in->Source->flag){
				links[sum++] = in->Source->idx;
				out->Source->flag = true;
			}
			in = in->down;
		}
	}
}
