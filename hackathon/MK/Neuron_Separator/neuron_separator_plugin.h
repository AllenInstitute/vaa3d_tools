/* Neuron_Separator_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2017-7-12 : by MK
 */
 
#ifndef __NEURON_SEPARATOR_PLUGIN_H__
#define __NEURON_SEPARATOR_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <qstringlist.h>
#include <qstring.h>
#include "basic_surf_objs.h"
#include <qvector.h>
#include <vector>
#include <map>

using namespace std;

struct nodeCheck
{
	NeuronSWC node;
	double dist;
	double dirIndex;
	double somaBranchNorm[3];
	int locOnPath, childLocOnPath;
	bool closerToHead;
};

struct somaNode // This structure carries crucial node properties on a soma tree and such.
{
	NeuronSWC node;
	vector<somaNode*> childrenSomas;
	vector<somaNode*> parent;
	somaNode* selfAddr;
	int level;
	bool branch, headSoma, tailSoma, middleSoma, soma;

	somaNode() 
	{
		branch = false; headSoma = false; tailSoma = false; middleSoma = false; soma = false;
		childrenSomas.clear();
		parent.clear();
	}
};

class neuronSeparator : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

	QString outputPath;
	
	/* -------------------------------------------------------------------------------------------------------------------------------------------- */
	neuronSeparator();
	
	vector<long int> somaIDs;
	QList<NeuronSWC> somaPath;
	NeuronTree somaSWCTree;
	
	// Soma tree building stuff ------
	QHash<long int, bool> locLabel;
		
	QList<NeuronSWC> findPath(QVector< QVector<V3DLONG> >& childList, NeuronTree& nt, long int wishedSomaID, long int excludedSomaID);
	long int findLatestCommonAncestor(bool& circle, NeuronTree& nt, QVector< QVector<V3DLONG> >& childList, NeuronSWC& wishedSoma, NeuronSWC& excludedSoma);
	void backwardPath(QList<NeuronSWC>& tracedSWC, NeuronTree& nt, NeuronSWC& start, NeuronSWC& end);
	void getMergedPath(QList<NeuronSWC>& somaPath, QHash<long int, bool>& locLabel, QList< QList<NeuronSWC> >& paths, NeuronTree& nt);
	// -------------------------------

	// Some hash methods -------
	QVector< QVector<V3DLONG> > childsTable; // childs table of the whole input SWC (in which hash neuron is available, given node's location -> the given node's children's location)
	
	QVector< QVector<V3DLONG> > childIndexTable(NeuronTree& nt); // this method produces the [childsTable]
	QVector< QVector<V3DLONG> > mkChildTableScratch(QList<NeuronSWC>&); // Same as childsTable, but is for simple swc that doesn't have hash neuron information (loc -> loc)
	vector<long int> mkPaTableScratch(QList<NeuronSWC>&); // given node's location -> the given node's parent's location  
	map<long int, size_t> hashScratchMap(QList<NeuronSWC>&); // ID -> location
	// -------------------------

	// Some tracing methods -------
	void downward(QList<NeuronSWC>& tracedSWC, QList<NeuronSWC>& inputList, NeuronSWC& start);
	QList<NeuronSWC> swcTrace(QList<NeuronSWC>& list, long int startID, NeuronSWC& startNode);
	// ----------------------------

	// Containers for soma path breaking stuff --------
	QList<NeuronSWC> brokenSomaPath;
	vector<long int> nodeToBeCutID;
	QList<NeuronSWC> brokenInputSWC;
	// ------------------------------------------------

	// Soma detection ---------------------------------
	vector<long int> autoSomaIDs;

	void autoSomaDetection();
	// ------------------------------------------------

protected:
	void buildSomaTree(); // This method produces [crucialNodes] and establishes hierarchical orders. 
	void breakPathMorph(somaNode* somaTreePtr); // This is the method that does the job => CUTS THE SOMA TREE AND PARTITIONS NEURONS
	long int pathScissor(QList<NeuronSWC>& segment); // This method gets called by breakSomaMorph.

private:
	bool circle;
	bool branchAncestor;
	int forward;
	NeuronTree inputSWCTree;
	QList< QList<NeuronSWC> > paths;

	somaNode* somaTreePtr; // the pointer to the head soma of the soma tree
	map<size_t, somaNode*> crucialNodeHash; // location on somaPath -> crucial node's STACK memory address
	vector<somaNode> crucialNodes; // SOMA TREE MEMBERS
};

#endif

