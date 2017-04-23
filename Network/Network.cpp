// Network.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "stdafx.h"
#include <stack>
#include "Network.h"

/*����ʮ�������ͼ�ṹ*/
using namespace std;

Node* Network::add_Node(int ID){
	/*
	����ID��ӽڵ㣬����ڵ��Ѿ���ӹ�����ֱ�ӷ���ָ��ýڵ��ָ��;
	�޸�IDToPointerӳ���
	*/
	hash_map<int, Node*>::iterator iter = IDToPointer.find(ID);
	if (iter != IDToPointer.end()){ // ���ID�Ѵ��ڣ���ֱ�ӷ���ָ��ýڵ��ָ��
		return iter->second;
	}
	if (Sum_of_Nodes >= Size){ //���ռ䳬���ѷ���ռ䣬���·����ڴ�ռ�
		if ((nodes = (Node*)realloc(nodes, Size + 500)) == NULL){
			return false;
		}
		Size += 500;
		int i = 0;
		for (iter = IDToPointer.begin(); iter != IDToPointer.end(); ++iter){
			iter->second = nodes + i; // �޸����нڵ��ָ��
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

	IDToPointer.insert(hash_map<int, Node*>::value_type(ID, nodes + Sum_of_Nodes)); // ��¼ID��ָ���Ӧ��ϵ

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

	/*��ӱߣ�������ָ��*/
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
	/*��������һ*/
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
	// �����·�����ƺ�ǰ���ڵ���г�ʼ��
	for (int i = 0; i < Sum_of_Nodes; i++){
		nodes[i].d = INT_MAX;
		nodes[i].pres = NULL;
	}
	s->d = 0;
}
void Network::relax(Edge* e, Priority_Queue* pri_q){
	// ��(u, v)�����ɳ�
	Node* u = e->Source;
	Node* v = e->Destination;
	double weight = e->Cost;

	Path_Link* s = v->pres;

	if (v->d > u->d + weight){
		pri_q->Decrease_Key(v, u->d + weight);
		// ɾ����¼ǰ���ڵ�Ŀ�
		while (s != NULL){
			Path_Link* temp = s->next_Link;
			delete s;
			s = temp;
		}
		Path_Link* pl = new Path_Link(); // �½�Path_Link����ʼ��
		pl->availableBandwidth = e->availableBandwidth;
		pl->next_Link = NULL;
		pl->pre_node = u;
		pl->flag = false;
		v->pres = pl;
	}
	else if (v->d == u->d + weight){
		Path_Link* pl = new Path_Link(); // �½�Path_Link����ʼ��
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
	//std::vector<Node> S; // �ڵ㼯��S
	Node* des = get_Node(destinationID);

	if (!s || !des) return;

	initialize_Single_Source(s); // ���ڴ�����С���ȶ���֮ǰ
	Priority_Queue* pri_q = new Priority_Queue(nodes, Sum_of_Nodes); //������С���ȶ���

	
	while (!pri_q->Empty()){
		Node* u = pri_q->Extract_Min();
		//S.push_back(u);
		// ��u��ÿ�����߽����ɳڲ���
		//cout << u->ID << endl << endl;
		if (u->ID == destinationID) break;
		if (u->d == INT_MAX) {
			//cout << ".";
			break;
		};
		Edge* e = u->out;
		while (e != NULL){
			if (e->availableBandwidth >= bandwidth) // ��������ҵ��Ҫ��
				relax(e, pri_q);
			e = e->right;
		}
		
	}

	/*�ҵ���ͼ�д�s��des���������·��*/
	vector<Path_Link*> pls;
	vector<Path_Link*> pls_v;// ��¼��ͼ�������s��des���������Path_Link�ͽڵ��ָ��
	while (true){
		// ��ʼ��
		for (int i = 0; i < Sum_of_Nodes; i++){
			nodes[i].flag = false;
		}
		/*��һ�δ�des��ǰ���ݣ���s->flagΪ�٣���δ�����ʣ��򲻴��ڴ�des��s��·��*/
		s->flag = false;
		pls.clear();
		DFS_Visit(des, s, pls); // �ҵ���������ͼ�о�����С�����Path_Link�ͽڵ��ָ��

		if (!s->flag) // ���s->flagΪ�٣���˵����������ͼ�е���С����ı��޷������յ㣬��ʱ����������Ӧ�Ĵ�����ҵ���������
			break;
		pls_v.clear();
		for (int i = 0; i < pls.size(); i++){// ������Щ��
			pls[i]->flag = true;
			pls_v.push_back(pls[i]);
		}
	}

	if (pls_v.empty()){ // û���������Ҫ���·��
		//s->pre = NULL;
		return;
	}
	for (int i = 0; i < pls_v.size(); i++){ // ȡ������(��Ϊ���һ���޷������յ�)
		pls_v[i]->flag = false;
	}

	/*�ҵ���С������·���������������л��޸Ľڵ��preָ�룬ʹ��ָ�������Сhops�Ľڵ�*/
	
	for (int i = 0; i < Sum_of_Nodes; i++){
		nodes[i].flag = false;
		nodes[i].hops = INT_MAX;
		nodes[i].pre = NULL;
	}
	des->hops = 0;
	min_Hops(des, s);

	/*���������ж�·���Ƿ����*/
	Node* no = s;
	while (no != des && no->pre != NULL){
		no = no->pre;
	}
	if (no != des){
		s->pre = NULL;
	}
	// �ͷ�Path_Link��
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

	// �ͷ���С���ȶ���
	delete pri_q;
}

void Network::DFS_Visit(Node* u, Node* s, vector<Path_Link*>& pls){
	/*
	��u��ʼ������CSPF��������ͼ���ҵ���������ͼ�д�����С�ıߵ�ָ�롣
	�ٴε��øú���ʱ����ԭ�������ϣ���������һ������ʱ���εıߣ����С�
	*/
	if (u == NULL || s == NULL) return;
	if (u->flag){
		return;
	}

	u->flag = true; // �ڵ�u�ѷ���
	//cout << u->ID << "-";
	if (u->ID == s->ID) {
		//cout << u->ID << " ";
		return;
	}
	Path_Link* pl = u->pres;
	if (pl == NULL) // û�б�
		return;
	Path_Link* pLink = NULL;
	while (pl != NULL){
		if (pl->flag){// ��������߱�����
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
	// ���ӽڵ�node���ھӽڵ���
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
	// ��ʼ����
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
		// ��ʼ��
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

		// ��ʼ����
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
