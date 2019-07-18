/*
wirtten by heping
2019/3/30
*/
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "genVirture_plugin.h"
#include<iostream>
#include<random>
#include<vector>
#include<queue>
#include<algorithm>

using namespace std;
#define Max_in 50000 
vector<edge> G1[Max_in];
vector<int> order;
priority_queue<edge> que1;
vector<edge> edges;
vector<int> num1;
default_random_engine eg;
//normal_distribution <float> distr();
uniform_real_distribution<double> unf(0.0, 1.0);

//��ʼ���������нڵ��ɢ���ҽ�����֮��ľ��뱣��ɾ���
void init(QList <NeuronSWC> &listNeuron){
	

}

void random_distr(){
	
}

void gen_axon(QList <NeuronSWC> &listNeuron){
	V3DLONG size = listNeuron.size();
	vector<bool> vis = vector<bool>(size, false);
	//init(listNeuron);
	vis[0] = true;
	num1 = vector<int>(size, 0);
	order = vector<int>(size, 0);
	cout << "generate axon!!!" << endl;
	cout << size << "------------"<<endl;
	QList <NeuronSWC> new_listNeuron;
	V3DLONG id=1;
	for (V3DLONG i = 0; i < size; i++){
		if (listNeuron[i].type == 3){//������ͻ
			listNeuron[i].n = id;
		//	listNeuron[i].parent = -1;
			new_listNeuron.append(listNeuron[i]);
			id++;
		}
	}
	listNeuron = new_listNeuron;
	size = listNeuron.size();
	cout << size << endl;
	//��ʼ�������е��������
	for (V3DLONG i = 0; i < size; i++){
		listNeuron[i].parent = -1;
	}
	for (int i = 0; i < size; i++){
		G1[i].clear();
	}
	//��������֮��ľ��룬�γɾ���
	while (que1.size())que1.pop();
	for (int i = 0; i < size; i++){
		for (int j = 0; j < size; j++){
			if (i == j)break;
			float cost = E_distance(listNeuron[i], listNeuron[j]);
			G1[i].push_back(edge(i + 1, j + 1, cost));//����յ��Ǳ��
			G1[j].push_back(edge(j + 1, i + 1, cost));

		}
	}
	//��һ������Ϊsoma
	for (int i = 0; i < G1[0].size(); i++){
		que1.push(G1[0][i]);
		//cout << G1[0][i].cost;
	}
	cout << G1[0].size();
	cout << "*****************" << endl;
	//���������⣬����
	//�����soma�ֲ��5-10����֦
	srand(1);
	//cout << rand();
	//int r = 0;
	int r = rand() % 6+5;
	cout << r << endl;
	for (int j = 0; j < r; j++){
		if (que1.size()){
			edge e = que1.top();
			que1.pop();
			listNeuron[e.to - 1].parent = e.from;//��������soma��
			cout << e.from << "-->" << e.to << endl;
			order[e.to - 1] = order[e.from - 1] + 1;//����˳��
			vis[e.to - 1] = true;
		}
	}
	cout << "111111111111" << endl;
	//����soma�������ʹ��Ľڵ����V��
	for (int i = 1; i < size; i++){
		if (vis[i]){
			for (int j = 0; j < G1[i].size(); j++)
				que1.push(G1[i][j]);
		}
	}
	cout << "generateing-----------" << endl;
	//
	while (que1.size()){
		edge e = que1.top();
		edge e1;
		bool flag = false;
		que1.pop();
		if (vis[e.to - 1])continue;
		if (num1[e.from - 1] != 0){//�Ѿ��������ֲ�����쳤��
			continue;
		}
		srand(order[e.from-1]);
		//float r = unf(e);
		float r = rand() % 100 / 100.0;
		cout << r << endl;
		//int r = un(order[e.from - 1]);
		//�ֲ�
		if (r > 0.6&&r<0.9){
			listNeuron[e.to - 1].parent = e.from;
			vis[e.to - 1] = true;
			order[e.to - 1] = order[e.from - 1] + 1;
//			cout << e.from << "-->" << e.to << endl;
			num1[e.from - 1] = 2;
			while (que1.size()){//
				 e1 = que1.top();
				 que1.pop();
				// cout << e1.from<<" ";
				if (e1.from == e.from&&!vis[e1.to-1]){
					listNeuron[e1.to - 1].parent = e.from;
					order[e1.to - 1] = order[e1.from - 1] + 1;					
					vis[e1.to - 1] = true;
//					cout << e1.from << "-->>" << e1.to << endl;
					flag = true;
					break;
				}
				else{
					edges.push_back(e1);
				}
			}		
			for (int i = 0; i < edges.size(); i++)
				que1.push(edges[i]);
		}	
		else if(r>=0.9){//ֹͣ����
			vis[e.to - 1] = true;
			listNeuron[e.to - 1].parent = e.from;
			order[e.to - 1] = order[e.from - 1] + 1;
			num1[e.from - 1] = 1;
			break;
		}
		//��������
		else{
			listNeuron[e.to - 1].parent = e.from;
//			cout << e.from << "-->" << e.to << endl;
			vis[e.to - 1] = true;
			order[e.to - 1] = order[e.from - 1] + 1;
			num1[e.from - 1] = 1;//�쳤��
		}
		//���¼���ĵ������ı߼����ѡ����,Ϊʲô����һ������ֶ��
		cout << num1[e.from - 1] << endl;
		
		for (int i = 0; i < G1[e.to - 1].size(); i++){
			que1.push(G1[e.to - 1][i]);
		}
		//�ֲ������ѡ����
		if (flag){
			for (int j = 0; j < G1[e1.to - 1].size(); j++){
				que1.push(G1[e1.to - 1][j]);
			}

		}
		
	}
	//ȷ�����е��Ƿ񶼷��ʹ�
	for (V3DLONG i = 0; i < size; i++){
		if (vis[i])continue;
		else{
			cout << "error----??" << endl;
			break;
		}
	}

}

void branch(){

}