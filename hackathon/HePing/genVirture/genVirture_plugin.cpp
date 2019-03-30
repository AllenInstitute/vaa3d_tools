/*
genVirture_plugin.cpp
Tue Mar 26 14:52:35 2019 by heping
*/

#include "v3d_message.h"
#include <vector>
#include "genVirture_plugin.h"
#include<iostream>
#include <QString>
#include <QtCore/QFile>

using namespace std;

Q_EXPORT_PLUGIN2(genVirture,genVirturePlugin);
void function(V3DPluginCallback2 &callback,QWidget *parent);
bool function(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent);
const QString title = QObject::tr("genVirture");
void generate_virture();
void configure();
bool generate_virture1(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent);//��ʱ�������������������ʱʹ�õĺ���
const QString filename1 = "D:/test1.swc";
const QString filename2 = "D:/test.swc";
const QStringList *infostring;
QStringList genVirturePlugin::menulist() const{//�˵�
	return QStringList()
		<< tr("generate_virture")//������Ԫ�˵�
		<< tr("configure");//�û���������һЩ�Զ�������ĵ�����Ŀǰ���� date 3/26
}

QStringList genVirturePlugin::funclist() const{//����
	return QStringList()
		<< tr("generate_virture");
}

void genVirturePlugin::domenu(const QString &menu_name,V3DPluginCallback2 &callback,QWidget *parent){
   if(menu_name==tr("generate_virture")){
       generate_virture();
   }
   else if (menu_name == tr("configure")){
	   configure();
   }
   else{
       v3d_msg(tr("developing...Tue Mar 26 14:52:35 2019 by heping"));
   }
}

bool genVirturePlugin::dofunc(const QString &func_name,const V3DPluginArgList &input,V3DPluginArgList &output,V3DPluginCallback2 &callback,QWidget *parent){
   if(func_name==tr("generate_virture")){
	   return generate_virture1(callback,input, output, parent);
   }
   else if(func_name==tr("help")){
       return true;
   }
   return false;
}

void generate_virture(){//��������������Ԫ�ĺ���	
	QStringList filenames = QFileDialog::getOpenFileNames(0, 0, "", "support file(*.swc)", 0, 0);
	QList <NeuronSWC> new_listNeuron_denr;
	QList <NeuronSWC> new_listNeuron_axon;
	if (filenames.isEmpty()){
		v3d_msg("You don't chose any swc file!");
		return;
	}
	if (filenames.size() != 1){
		v3d_msg("You chose too many swc file!");//Ŀǰֻѡ��һ��swc�ļ�����������Ԫ
		return;
	}
	NeuronTree ntree = readSWC_file(filenames[0]);
	V3DLONG neuron_size = ntree.listNeuron.size();
	vector <V3DLONG> nodes=vector<V3DLONG>(neuron_size,0);
//	vector <V3DLONG> nodes_axon=vector<V3DLONG>(neuron_size,0);
	NeuronSWC seed;//SWC�ļ��е�һ�������soma
	seed = ntree.listNeuron[0];
	cout << "----------------------" << endl;
	
	//����ͻ����ͻ�ֿ�������ڵ�ĺ��ӽڵ�����
	calculate_nodes(ntree, nodes);
	//����ͻ�Ĺؼ��ڵ㱣������,����ͻ�Ĺؼ��ڵ㱣������
    save_import_nodes(ntree,nodes,new_listNeuron_denr,new_listNeuron_axon);
	cout << "save  finish!" << endl;
	cout << new_listNeuron_axon.size() <<"--------" <<new_listNeuron_denr.size() << endl;
	cout << "--------------------------" << endl;
	//��С�������㷨prim�����ӵ�ı�Ż���1
	min_tree_prim(new_listNeuron_denr);
	min_tree_prim(new_listNeuron_axon);
	ntree.listNeuron = new_listNeuron_denr;//hashNeuron��֪��Ҫ��Ҫ�޸�
	cout << "wirte swc--------------------" << endl;
	
	if (writeSWC_file(filename1, ntree, infostring)){
		v3d_msg("finish it");
	}
	else{
		cout << "write error!!!!" << endl;
	}

	ntree.listNeuron = new_listNeuron_axon;//hashNeuron��֪��Ҫ��Ҫ�޸�
	if (writeSWC_file(filename2, ntree, infostring)){
		v3d_msg("finish it");
	}
	else{
		cout << "write error!!!!" << endl;
	}
	//���߳ɱ�����·���ɱ�
	//��������Ӱ�죺����Ũ�ȡ����������------------�ݲ�����
	//�ε�ֱ���𽥼�С-------�ݲ�����
}

void configure(){};//��ʱ���ܲ������ã����ù̶�����ĳЩ����

bool generate_virture1(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent){//��ʱ����
   vector<char*> *pinfiles=(input.size()>=1)?(vector<char*> *) input[0].p : 0;
   vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
   vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
   vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
   vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
   vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
   return true;
}

