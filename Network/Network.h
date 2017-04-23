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
#include <sstream>    //使用stringstream需要引入这个头文件 
#include<time.h>
using namespace std;
// 声明
typedef enum Type{ INDEX, ID } Type;
typedef struct Edge Edge;
typedef struct Node Node;
typedef struct Path_Link Path_Link;
class Graph;
class Priority_Queue;


// 图边定义
typedef struct Edge{
	int LinkID; // 链路ID
	bool flag; // 该边是否访问过
	Node* Source; // 指向源节点的指针
	Node* Destination; // 指向目的节点的指针
	double Cost; // 链路代价
	double Bandwidth; //链路带宽
	double availableBandwidth;// 可用链路带宽
	Edge* down; // 指向与该边目的节点相同的下一条边
	Edge* right; // 指向与该边源节点相同的下一条边
}Edge;

// 表头定义
typedef struct Node{
	int ID; // 节点ID
	int community; // 记录节点属于的社团号
	double d;// 用来记录从源节点u到节点v的最短路径权重的上界，称d为s到v的最短路径估计
	bool flag; // 用来记录该节点是否已经被访问过
	int hops; // 指示最小跳数
	int pointer_index; // 指示最小优先队列中指针数组的对应下标
	int idx;//  nodes中节点对应下标
	Node* pre; // 记录最佳路径
	Path_Link* pres; //指向前驱节点的指针,组成链表
	Edge* in;// 指向以该节点为目的节点的第一条边
	Edge* out; // 指向以该节点为源节点的第一条边

}Node;

// 用来保存最佳路径上的节点
typedef struct Path_Link{
	double availableBandwidth; // 指示这条路径的可用带宽
	bool flag; // 指示是否已经屏蔽这条边
	Node* pre_node; // 指示代价相同的路径的上一节点
	Path_Link* next_Link; // 指示另一条代价相同的路径
}Path_Link;

/*最小优先队列*/
class Priority_Queue{
	Node** H; // 指向指针数组的指针，其元素为指向Node型对象的指针
public:
	Priority_Queue(Node* nodes, int n){
		H = new Node*[n + 1];
		heap_size = n;

		for (int i = 1; i <= n; i++){
			H[i] = nodes + i - 1; //初始化，依次将指向node型对象的指针存入指针数组中
			H[i]->pointer_index = i; // 保存指向nodes[i-1]的指针的下标，以方便在Find_Idx中查找
		}
		// 建最小堆
		for (int i = n / 2; i >= 1; i--){
			Min_Heapify(i);
		}
	}
	~Priority_Queue(){
		//释放指针数组
		delete[] H;
		H = NULL;
	}
	Node* Extract_Min(){
		// 去掉并返回H中具有最小值的元素
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
		// 将指针a指向元素的d值减小到new_d，这里假设new_d不大于原来d值
		int i = a->pointer_index;
		if (i == 0) {
			//cout << "索引为0."<<endl;
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
	int heap_size = 0; // 堆大小
	/*给定一个结点的下标i，计算它的父结点、左孩子和右孩子的下标*/
	int Parent(int i){ return i >> 1; }
	int Left(int i){ return i << 1; }
	int Right(int i){ return (i << 1) + 1; }
	void exchange(Node* &a, Node* &b){
		// 实现指针a和b的交换
		Node* temp = a;
		a = b;
		b = temp;
	}

	void Min_Heapify(int i){
		// 堆调整
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
		/*使用循环控制结构取代递归，重写代码*/
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
		if ((nodes = (Node*)malloc(Size * sizeof(Node))) == NULL){ // 预分配10000个Node空间大小的内存区
			exit(0);
		}
	}
	
	//friend class Priority_Queue; // 类Priority_Queue是类Graph的友元
	Node* add_Node(int);
	bool add_Edge(Edge*, int, int);
	Node* get_Node(int, Type);
	Edge* get_Edge(int, int);
	void CSPF(int, int, double);
	int getSumofNodes(){ return Sum_of_Nodes; }
	int getSumofEdges(){ return Sum_of_Edges; }
	int nb_In_Neighbors(Node*); // 返回节点的入度
	int nb_Out_Neighbors(Node*); // 返回节点的出度
	int nb_Neighbors(Node* u); // 返回与节点u相邻的节点数
	int undirected_Weight(Node*, Node*); // 计算两节点之间的权重
	int weighted_Degree(Node*); // 计算节点的带权度数
	int nb_Undirected_Links(); // 计算图中节点相邻的无向边数（2条有向边算一条无向边，权重为2）
	double cal_Total_Weight();
	void undirected_Links(int*);
	
	~Network(){
		// 释放边占用的内存空间
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
		// 释放节点占用的内存空间
		free(nodes);
		nodes = NULL;
	}
private:
	Node* nodes; // 存储Node
	hash_map<int, Node*> IDToPointer;
	int Sum_of_Nodes = 0; // 节点总数
	int Sum_of_Edges = 0; //边总数
	int Size; // 分配存储空间大小
	double total_weight = 0; //总权重
	bool graph_Change = false; // 图结构是否发生变化
	int links = 0;

	void initialize_Single_Source(Node*); // 对最短路径估计和前驱节点进行初始化
	void relax(Edge*, Priority_Queue*);// 对边（u, v）进行松弛操作 
	void DFS_Visit(Node* u, Node* s, vector<Path_Link*>& pls); // 从节点u开始深度遍历路径,寻找满足带宽的路径,并返回路径图中最小带宽的path_Link型节点，到s停止
	void min_Hops(Node* u, Node*); // 计算由CSPF产生的子图的最小跳数的路径
};


#endif