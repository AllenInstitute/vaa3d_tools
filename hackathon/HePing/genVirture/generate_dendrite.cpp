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
vector<edge> G[Max_in];
//��С�������㷨
void min_tree_prim(QList <NeuronSWC> &new_listNeuron){//���е�����,�����н���������x,y,z����������������,�ֲ�һ�������������,ʲôʱ����ֹ
	//soma����list�е�һ��
	V3DLONG size = new_listNeuron.size();
	cout <<"start distance" << endl;	
	float min_dist1, min_dist2;
//	vector <float> dista;	
//	int flag1,flag2=0;
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

	//�ֿ���ͻ����ͻ�Ĳ��֣���ͻֻ��һ�����ɵ�cluster�����ѡ������ĵ���Ϊ��ͻ����ͻ���ӵĵ�
//	for (V3DLONG i = 0; i < size; i++){//----------
//		cout << "start distance" << endl;
//		for (int j = i; j <size; j++){
//			cout << (2 * size - i + 1)*i / 2 + j - i << "!!";
//			if (i == j){
//				dista.push_back(0);//ֻ����push_back��VECTORĩβ�����ֵ������ֱ����û�з���ռ�ĵ�ַ����[]����ֵ
				
//			}				
//			else{
//				dista.push_back(E_distance(new_listNeuron[i], new_listNeuron[j]));				
//			}				
//		}
//	}


	
	//ѡ�������һ������������㣬������һֱ���ӣ�ѭ��100�ν���
	
//	vector<V3DLONG> d;
//	d.push_back(0);
//	for (V3DLONG k = 0; k < size-1; k++){
		
//		cout << "start recurrent----"<<k << endl;
//		min_dist1 = dista[(2*size-k+1)*k/2 + 2];//ȡ��������������ʼ��
//		min_dist2 = dista[(2 * size - k + 1)*k / 2 + 3];
//		cout << min_dist1 << "--" << min_dist2 << endl;
//		V3DLONG j;
//		flag1 = k+1;
//		//flag2 = k+2;
//		vis[flag1] = true;
		//vis[flag2] = true;
//		count = 0;
//		for (j = k+1; j < size; j++){
//			if (count > 50) break;
// 	 	    if (dista[(2 * size - k + 1)*k / 2 + j - k] < min_dist1&&vis[j]==false){
//				min_dist1 = dista[(2 * size - k + 1)*k / 2 + j - k];
//				cout << min_dist1<<"--";
//				vis[flag1] = false;
//				flag1 = j;				
//				vis[j] = true;
//			}								
//			else { 
//				count++;
//				continue; }
//		}
		
		//srand(k);
		//float ra=rand() % 100 / double(100);//����0-1��Χ�ڵ������
		//if (ra > 0.6){//����һ����ֵ�ֲ棬���ڲ���Ҫ----Ŀǰֻ�����ֲ��
//
		//}
//		cout << endl;
///		cout << flag1 << "#" << endl;
//		new_listNeuron[flag1].parent = k+1;
	//	new_listNeuron[flag2].parent = k + 1;
//	}
	
	cout << "end" << endl;
}

//prim_dendrite
void prim(QList <NeuronSWC> &new_listNeuron){
	V3DLONG size = new_listNeuron.size();
	int count;
	//prim  -------------���Ʒֲ����-------------
	float sum_cost;
	vector<bool> vis = vector <bool>(size, false);
	vector<int> num = vector<int>(size, 0);
	//������1��Ϊ��ʼ��
	vis[0] = true;
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

//��SWC�ļ������нڵ�ĺ��ӽڵ������������,������ͻ����ͻ�ֿ�
void calculate_nodes(NeuronTree ntree, vector<V3DLONG> &nodes){
//	QMap <int, int>  mapDenrite;
//	QMap <int, int>  mapAxon;	
	V3DLONG neuron_size = ntree.listNeuron.size();
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
		//	if (mapAxon.value(nid)==0){
		//		mapAxon.insert(nid, aid);				
				
	//		}
	//		else{
	//			aid = mapAxon.value(nid);
	//		}
			//����nodes_axon�±���listNeuron��Ӧ�Ľڵ��±�
	//		nodes_axon[aid]++;
	//		cout << "parent:"<<nid <<"aid:"<< aid <<"num:"<< nodes_axon[aid] << endl;
	//		aid++;			

		}
		
	//		if (mapDenrite.value(nid) == 0){
	//			mapDenrite.insert(nid, did);
	//			
	//		}
	//		else{
	//			did = mapDenrite.value(nid);
	//		}
			//����nodes_axon�±���listNeuron��Ӧ�Ľڵ��±�
	//		nodes_denr[did]++;
	//		cout << "parent:"<<nid <<"did:"<< did <<"num:"<< nodes_denr[did] << endl;
	//		did++;
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
			if (ntree.listNeuron.at(i).type == 2){//��ͻ
				new_listNeuron_axon.append(ntree.listNeuron.at(i));
				new_listNeuron_axon[noa].n = noa + 1;//�ڵ����±��,��Ŵ�1��ʼ
				new_listNeuron_axon[noa].parent = -1;//ȫ����ʼ���޸��ڵ�
				cout << new_listNeuron_axon[noa].n << "---";
				noa++;
			}
			else{//��ͻ
				new_listNeuron_denr.append(ntree.listNeuron.at(i));
				new_listNeuron_denr[nod].n = nod + 1;//�ڵ����±�ţ���Ŵ�1��ʼ
				new_listNeuron_denr[nod].parent = -1;//ȫ����ʼ���޸��ڵ�
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


//·���ɱ�
void cost_path(){

}