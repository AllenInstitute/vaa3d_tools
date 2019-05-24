/*
wirtten by heping
2019/3/26
*/
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "genVirture_plugin.h"
#include <vector>
#include<stdlib.h>
#include<iostream>
#include<queue>
using namespace std;
#define Max_in 500
priority_queue<edge> que;
vector<edge> G[Max_in];//��������֮��ľ���
vector<float> path_cost;//ÿ�����·������
float PATH_LENGTH;

//��С�������㷨
void min_tree_prim(QList <NeuronSWC> &new_listNeuron){//���е�����,�����н���������x,y,z����������������,�ֲ�һ�������������,ʲôʱ����ֹ
	//soma����list�е�һ��
	V3DLONG size = new_listNeuron.size();
	cout <<"start distance" << endl;	
	float min_dist1, min_dist2;
	cout << size << endl;
	//��ʼ��G
	for (int i = 0; i < size; i++){
		G[i].clear();
	}
	
	while (que.size())que.pop();
	for (int i = 0; i < size; i++){
		for (int j = 0; j < size; j++){
			if (i == j)break;
			float cost = E_distance(new_listNeuron[i], new_listNeuron[j]);
			G[i].push_back(edge(i+1,j+1, cost));//����յ��Ǳ��
			G[j].push_back(edge(j+1,i+1, cost));
			
		}
	}

	prim(new_listNeuron);
	cout << "end" << endl;
}

//prim_dendrite
void prim(QList <NeuronSWC> &new_listNeuron){
	V3DLONG size = new_listNeuron.size();
	vector<float> wring_cost=vector<float>(size,0.0);
	int count;
	//prim  -------------���Ʒֲ����-------------
	float sum_cost;
	vector<bool> vis = vector <bool>(size, false);
	vector<int> num = vector<int>(size, 0);
	//������1��Ϊ��ʼ��
	vis[0] = true;
	wring_cost[0] = 0;//soma��
	for (int i = 0; i < G[0].size(); i++){
		que.push(G[0][i]);
		cout << G[0][i].cost;
	}
	cout << endl;
	srand(que.size());
	int a = rand() % 4 + 4;
	count = 0;
	//��soma�����ҵ�4-8���ӽڵ�����,�����prim��������һ��
	while (que.size()){
		count++;
		if (count > a)break;
		else{
			edge e = que.top();
			que.pop();
			if (vis[e.to - 1])continue;
			vis[e.to - 1] = true;
			new_listNeuron[e.to - 1].parent = e.from;
			//wring_cost[e.to - 1] = wring_cost[e.from - 1] + e.cost;//ÿ�����,�Ӹ��ڵ㵽�õ��·������
			//PATH_LENGTH += wring_cost[e.to - 1];//����·������
			cout << e.from << "----" << e.to << endl;
			sum_cost += e.cost;
		}
	}
	//����soma�������ʹ��Ľڵ����V��
	for (int i = 1; i < size; i++){
		if (vis[i]){
			for (int j = 0; j < G[i].size(); j++)
				que.push(G[i][j]);
		}
	}
	//ÿ��ѡȡV��S-V������·����̵ı�
	while (que.size()){
		edge e = que.top();
		que.pop();
		if (vis[e.to - 1])continue;//�Ѿ����ʹ��ģ�e.to����һ����ı�ţ�����λ��=���-1
		if (num[e.from - 1] < 2){//���ֻ��������֧
			vis[e.to - 1] = true;
			new_listNeuron[e.to - 1].parent = e.from;
			sum_cost += e.cost;
			num[e.from - 1]++;//�ýڵ�ĺ��Ӹ���
			//wring_cost[e.to - 1] = wring_cost[e.from - 1] + e.cost;
			//PATH_LENGTH += wring_cost[e.to - 1];
			cout << e.from << "----" << e.to << endl;
			cout << e.cost << endl;
			for (int j = 0; j < G[e.to - 1].size(); j++){
				que.push(G[e.to - 1][j]);
			}
		}

	}
	//����ȷ���Ƿ����н�㶼���ʹ���ͼ�Ƿ���ͨ
	for (int i = 0; i < size; i++){
		cout << num[i] << " ";
		if (!vis[i])
			cout << i << endl;
	}
}

//һ�Խڵ�֮��ĵ�λ��������
NodeXYZ direction(NeuronSWC &Node1, NeuronSWC &Node2){
	NodeXYZ dire;
	float x1, y1, z1;
	float x2, y2, z2;
	float distance= E_distance(Node1, Node2);
	x1 = Node1.x;
	y1 = Node1.y;
	z1 = Node1.z;
	x2 = Node2.x;
	y2 = Node2.y;
	z2 = Node2.y;
	dire.x = (x2 - x1) / distance;
	dire.y = (y2 - y1) / distance;
	dire.z = (z2 - z1) / distance;
	return dire;

}
//һ�Խڵ�֮���ŷʽ����
float E_distance(NeuronSWC &Node1, NeuronSWC &Node2){//������֮���ŷʽ����
	float dist;
	float x1, y1, z1;
	float x2, y2, z2;
	x1 = Node1.x;
	y1 = Node1.y;
	z1 = Node1.z;
	x2 = Node2.x;
	y2 = Node2.y;
	z2 = Node2.y;
	dist = sqrt(pow(x1 - x2, 2)+pow(y1 - y2, 2)+pow(z1 - z2, 2));
	
	return dist;

}

//��SWC�ļ������нڵ�ĺ��ӽڵ������������
void calculate_nodes(NeuronTree ntree, vector<V3DLONG> &nodes){
//	QMap <int, int>  mapDenrite;
//	QMap <int, int>  mapAxon;	
	V3DLONG neuron_size = ntree.listNeuron.size();
	//��Ӧλ�ú��ӵĸ���
	nodes = vector<V3DLONG>(neuron_size, 0);
	cout << neuron_size;
	//ͳ�����нڵ�ĺ��ӽڵ�����
	cout << "start calculate nodes number---" << endl;
//	int did=1, aid=1;
	//V3DLONG no = 0;
//	mapAxon.clear();
//	mapDenrite.clear();
	for (V3DLONG i = 1; i < neuron_size; i++){
		
		if (ntree.listNeuron[i].type == 2 || ntree.listNeuron[i].type == 3){//��ͻ�����ڲ�ͬ�Ľṹ�����㷨�Ƿ���Ҫ��ͬ--------
			V3DLONG nid = ntree.hashNeuron.value(ntree.listNeuron[i].parent);//����list�еĸýڵ�ĸ��ڵ���list�е�λ��id��������
			nodes[nid]++;			

		}
		else continue;
		
	}
//	cout << "axon nodes num"<<nodes_axon.size();
//	cout <<"denrite nodes num"<< nodes_denr.size();
	cout << "finish it!" << endl;
	
}

//����ؼ��ڵ�
void save_import_nodes(NeuronTree &ntree, vector<V3DLONG> &nodes, QList <NeuronSWC> &new_listNeuron_denr, QList <NeuronSWC> &new_listNeuron_axon){
	
	V3DLONG size = nodes.size();
	int noa = 0;
	int nod = 0;
	for (V3DLONG i = 0; i < size; i++){
		cout << nodes[i] << "-";
		if (nodes[i] != 1){//�ն˽ڵ㣬����
			if (ntree.listNeuron.at(i).type == 2){//��ͻ,��soma
				V3DLONG pn = ntree.listNeuron.at(i).parent;
				while (nodes[pn-1] == 1){
					pn = ntree.listNeuron.at(pn-1).parent;
				}
				
				new_listNeuron_axon.append(ntree.listNeuron.at(i));
				//new_listNeuron_axon[noa].n = noa + 1;//�ڵ����±��,��Ŵ�1��ʼ	
				new_listNeuron_axon[noa].parent = pn;
				
				//new_listNeuron_axon[noa].parent = -1;//ȫ����ʼ���޸��ڵ�
				cout << new_listNeuron_axon[noa].n << "---";
				noa++;
			}
			else{//��ͻ,����soma
				V3DLONG pn = ntree.listNeuron.at(i).parent;
				while (nodes[pn - 1] == 1){
					pn = ntree.listNeuron.at(pn - 1).parent;
					if (pn == 1)break;
				}
				new_listNeuron_denr.append(ntree.listNeuron.at(i));
				//new_listNeuron_denr[nod].n = nod + 1;//�ڵ����±�ţ���Ŵ�1��ʼ
				//new_listNeuron_denr[nod].parent = -1;//ȫ����ʼ���޸��ڵ�
				new_listNeuron_denr[nod].parent = pn;
				cout << new_listNeuron_denr[nod].n << "---";
				nod++;
			}			
			
		}		
		else {//�ֲ�㣬�������ֲ�ɼ����ӽڵ㣬��ʱ��ɾ������������֧��
			continue;
		}
		cout << endl;
	}
	

}

void save_point_nodes(NeuronTree &ntree, vector<V3DLONG> &nodes, QList <NeuronSWC> &new_listNeuron){
	V3DLONG size = nodes.size();
	int no = 0;
	for (V3DLONG i = 0; i < size; i++){
		if (nodes[i] != 1){//�ն˽ڵ㣬����
			V3DLONG pn = ntree.listNeuron.at(i).parent;
			while (nodes[pn - 1] == 1)
				pn = ntree.listNeuron.at(pn - 1).parent;
			new_listNeuron.append(ntree.listNeuron.at(i));
			new_listNeuron[no].parent = pn;
			no++;

		}
		else continue;
	}
}

NeuronTree ntree1;//�µ���
//·���ɱ�,�Ӹ��ڵ㵽�õ��·������
float cost_path(NeuronSWC Node){
	NeuronSWC node = Node;
	V3DLONG pn = node.parent;
	//·������
	if (pn = -1)
		return 0;
	else{
		NeuronSWC parent = ntree1.listNeuron.at(pn);
		return cost_path(parent) + E_distance(parent, node);
	}
}