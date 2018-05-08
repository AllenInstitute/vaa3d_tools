#ifndef NEURONSTRUCTEXPLORER_H
#define NEURONSTRUCTEXPLORER_H

#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <string>

#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

#include "v_neuronswc.h"

#include "basic_surf_objs.h"

using namespace std;

class NeuronStructExplorer
{
public:
	/********* Constructors and basic data members *********/
	NeuronStructExplorer() {};
	NeuronStructExplorer(string neuronFileName);

	NeuronTree* singleTreePtr;
	NeuronTree singleTree;
	NeuronTree processedTree;
	vector<NeuronTree>* treeVectorPtr;
	QString neuronFileName;
	/*******************************************************/

	/********* Pixel-based deep neural network result refining/cleaning *********/
	unordered_map<string, unordered_map<int, float>> zProfileMap;
	void detectedPixelStackZProfile(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr);
	void pixelStackZcleanup(unordered_map<string, unordered_map<int, float>> zProfileMap, NeuronTree* outputTreePtr, int minSecNum, bool max, int threshold = 0);
	/****************************************************************************/

	/********* Distance-based SWC analysis *********/
	vector<vector<float>> FPsList;
	vector<vector<float>> FNsList;
	void falsePositiveList(NeuronTree* detectedTreePtr, NeuronTree* manualTreePtr, float distThreshold = 20);
	void falseNegativeList(NeuronTree* detectedTreePtr, NeuronTree* manualTreePtr, float distThreshold = 20);
	void detectedDist(NeuronTree* inputTreePtr1, NeuronTree* inputTreePtr2);

	map<int, long int> nodeDistCDF;
	map<int, long int> nodeDistPDF;
	void shortestDistCDF(NeuronTree* inputTreePtr1, NeuronTree* inputTreePtr2, int upperBound, int binNum = 500);
	/*********************************/

	/********* Segment <-> Image assessment functionalities *********/
	V_NeuronSWC_list segmentList;
	void segmentDecompose(NeuronTree* inputTreePtr);
	/**************************************************************/
};

#endif