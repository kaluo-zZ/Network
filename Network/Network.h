#ifndef NETWORK_H
#define NETWORK_H

#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include <queue>
#include <vector>
#include <map>
#include <hash_map>
#include <sstream>    //ʹ��stringstream��Ҫ�������ͷ�ļ� 
#include<time.h>
using namespace std;
// ����
typedef enum Type{ INDEX, ID } Type;
typedef struct Edge Edge;
typedef struct Node Node;
typedef struct Path_Link Path_Link;
class Graph;
class Priority_Queue;


// ͼ�߶���
typedef struct Edge{
	int LinkID; // ��·ID
	bool flag; // �ñ��Ƿ���ʹ�
	Node* Source; // ָ��Դ�ڵ��ָ��
	Node* Destination; // ָ��Ŀ�Ľڵ��ָ��
	double Cost; // ��·����
	double Bandwidth; //��·����
	double availableBandwidth;// ������·����
	Edge* down; // ָ����ñ�Ŀ�Ľڵ���ͬ����һ����
	Edge* right; // ָ����ñ�Դ�ڵ���ͬ����һ����
}Edge;

// ��ͷ����
typedef struct Node{
	int ID; // �ڵ�ID
	int community; // ��¼�ڵ����ڵ����ź�
	double d;// ������¼��Դ�ڵ�u���ڵ�v�����·��Ȩ�ص��Ͻ磬��dΪs��v�����·������
	bool flag; // ������¼�ýڵ��Ƿ��Ѿ������ʹ�
	int hops; // ָʾ��С����
	int pointer_index; // ָʾ��С���ȶ�����ָ������Ķ�Ӧ�±�
	int idx;//  nodes�нڵ��Ӧ�±�
	Node* pre; // ��¼���·��
	Path_Link* pres; //ָ��ǰ���ڵ��ָ��,�������
	Edge* in;// ָ���Ըýڵ�ΪĿ�Ľڵ�ĵ�һ����
	Edge* out; // ָ���Ըýڵ�ΪԴ�ڵ�ĵ�һ����

}Node;

// �����������·���ϵĽڵ�
typedef struct Path_Link{
	double availableBandwidth; // ָʾ����·���Ŀ��ô���
	bool flag; // ָʾ�Ƿ��Ѿ�����������
	Node* pre_node; // ָʾ������ͬ��·������һ�ڵ�
	Path_Link* next_Link; // ָʾ��һ��������ͬ��·��
}Path_Link;

/*��С���ȶ���*/
class Priority_Queue{
	Node** H; // ָ��ָ�������ָ�룬��Ԫ��Ϊָ��Node�Ͷ����ָ��
public:
	Priority_Queue(Node* nodes, int n){
		H = new Node*[n + 1];
		heap_size = n;

		for (int i = 1; i <= n; i++){
			H[i] = nodes + i - 1; //��ʼ�������ν�ָ��node�Ͷ����ָ�����ָ��������
			H[i]->pointer_index = i; // ����ָ��nodes[i-1]��ָ����±꣬�Է�����Find_Idx�в���
		}
		// ����С��
		for (int i = n / 2; i >= 1; i--){
			Min_Heapify(i);
		}
	}
	~Priority_Queue(){
		//�ͷ�ָ������
		delete[] H;
		H = NULL;
	}
	Node* Extract_Min(){
		// ȥ��������H�о�����Сֵ��Ԫ��
		if (heap_size < 1){
			return NULL;
		}
		Node* min = H[1];
		H[1] = H[heap_size];
		H[1]->pointer_index = 1;
		heap_size -= 1;
		Min_Heapify(1);
		return min;
	}
	void Decrease_Key(Node*a, double new_d){
		// ��ָ��aָ��Ԫ�ص�dֵ��С��new_d���������new_d������ԭ��dֵ
		int i = a->pointer_index;
		if (i == 0) {
			//cout << "����Ϊ0."<<endl;
			return;
		}
		if (new_d > H[i]->d) return;
		H[i]->d = new_d;
		while (i > 1 && H[Parent(i)]->d > H[i]->d){
			H[i]->pointer_index = Parent(i);
			H[Parent(i)]->pointer_index = i;
			exchange(H[i], H[Parent(i)]);
			i = Parent(i);
		}
	}
	bool Empty(){
		if (heap_size == 0)return true;
		else return false;
	}

private:
	int heap_size = 0; // �Ѵ�С
	/*����һ�������±�i���������ĸ���㡢���Ӻ��Һ��ӵ��±�*/
	int Parent(int i){ return i >> 1; }
	int Left(int i){ return i << 1; }
	int Right(int i){ return (i << 1) + 1; }
	void exchange(Node* &a, Node* &b){
		// ʵ��ָ��a��b�Ľ���
		Node* temp = a;
		a = b;
		b = temp;
	}

	void Min_Heapify(int i){
		// �ѵ���
		/*
		int l = Left(i);
		int r = Right(i);
		int largest = 0;

		if (l <= heap_size && H[l]->d < H[i]->d){
		largest = l;
		}
		else{
		largest = i;
		}
		if (r <= heap_size && H[r]->d < H[largest]->d){
		largest = r;
		}
		if (largest != i){
		H[i]->pointer_index = largest;
		H[largest]->pointer_index = i;
		exchange(H[i], H[largest]);
		Min_Heapify(largest);
		}
		*/
		/*ʹ��ѭ�����ƽṹȡ���ݹ飬��д����*/
		while (true){
			int l = Left(i);
			int r = Right(i);
			int largest = i;
			if (l <= heap_size && H[l]->d < H[i]->d){
				largest = l;
			}
			if (r <= heap_size && H[r]->d < H[largest]->d){
				largest = r;
			}
			if (largest != i){
				H[i]->pointer_index = largest;
				H[largest]->pointer_index = i;
				exchange(H[i], H[largest]);
				i = largest;
			}
			else{
				break;
			}
		}
	}
};


class Network{
public:
	Network(){
		Size = 10000;
		if ((nodes = (Node*)malloc(Size * sizeof(Node))) == NULL){ // Ԥ����10000��Node�ռ��С���ڴ���
			exit(0);
		}
	}
	
	//friend class Priority_Queue; // ��Priority_Queue����Graph����Ԫ
	Node* add_Node(int);
	bool add_Edge(Edge*, int, int);
	Node* get_Node(int, Type);
	Edge* get_Edge(int, int);
	void CSPF(int, int, double);
	int getSumofNodes(){ return Sum_of_Nodes; }
	int getSumofEdges(){ return Sum_of_Edges; }
	int nb_In_Neighbors(Node*); // ���ؽڵ�����
	int nb_Out_Neighbors(Node*); // ���ؽڵ�ĳ���
	int nb_Neighbors(Node* u); // ������ڵ�u���ڵĽڵ���
	int undirected_Weight(Node*, Node*); // �������ڵ�֮���Ȩ��
	int weighted_Degree(Node*); // ����ڵ�Ĵ�Ȩ����
	int nb_Undirected_Links(); // ����ͼ�нڵ����ڵ����������2���������һ������ߣ�Ȩ��Ϊ2��
	double cal_Total_Weight();
	void undirected_Links(int*);
	
	~Network(){
		// �ͷű�ռ�õ��ڴ�ռ�
		Edge* e = NULL;
		for (int i = 0; i < Sum_of_Nodes; i++){
			e = nodes[i].out;
			nodes[i].out = NULL;
			while (e != NULL){
				Edge* temp = e->right;
				delete e;
				e = temp;
			}
		}
		// �ͷŽڵ�ռ�õ��ڴ�ռ�
		free(nodes);
		nodes = NULL;
	}
private:
	Node* nodes; // �洢Node
	hash_map<int, Node*> IDToPointer;
	int Sum_of_Nodes = 0; // �ڵ�����
	int Sum_of_Edges = 0; //������
	int Size; // ����洢�ռ��С
	double total_weight = 0; //��Ȩ��
	bool graph_Change = false; // ͼ�ṹ�Ƿ����仯
	int links = 0;

	void initialize_Single_Source(Node*); // �����·�����ƺ�ǰ���ڵ���г�ʼ��
	void relax(Edge*, Priority_Queue*);// �Աߣ�u, v�������ɳڲ��� 
	void DFS_Visit(Node* u, Node* s, vector<Path_Link*>& pls); // �ӽڵ�u��ʼ��ȱ���·��,Ѱ����������·��,������·��ͼ����С�����path_Link�ͽڵ㣬��sֹͣ
	void min_Hops(Node* u, Node*); // ������CSPF��������ͼ����С������·��
};


#endif