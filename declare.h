/*
 * declare.h
 *
 *  Created on: Mar 31, 2017
 *      Author: aa
 */





#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <iterator>
#include <iostream>

using namespace std;


struct interval{
	double left;
	double right;
	double loss;
};

struct predict{
	double treatment;
	double left;
	double right;
	double outcome;
	double loss;
};

struct splitFeature{
	string name;//The name of split attribute
	int type=1;//The type of split attribute
	double split_value;//the split value of split attribute if it is numerical. zero if it is nominal
	int vindex;//The index of split attribute in the attr_table
	double before,after; // the loss before and after
	vector<int> leftindex,rightindex;
};


struct Node{
	vector<int> index;
	unsigned depth=0;
	bool isLeaf=false;
	splitFeature spf;
	interval iv;
	vector<Node*> Children;
};

struct pruneBox{
	double reduction=0;
	unsigned size=0;
	double score;
	Node* node;
};

struct Model{
	Node* node;
	double cvLoss;
	double testLoss;
	double complexity;
};


struct attribute
{
	string name;
	int vindex;
	int type=1;// 1 for numeric and 0 for categorical
	vector<double> value;
	vector<double> breaks;
	double upper;
	double lower;
	bool discrete=false;
	vector<double> levels;
};

struct splitData{
	double value;
	int index;
};
extern unsigned sided;
extern double S;
extern double alpha;
extern unsigned nfolds;
extern double complexity;
extern unsigned maxDepth;
extern unsigned minSize;
extern double S;
extern double alpha;
extern vector<attribute> features;
//extern attribute outcome;
//extern attribute treatment;
extern vector<double> grid;

Model* cvTree(vector<vector<double> >&data,vector<vector<double> >&test,vector<vector<double> >&IndvLoss,vector<double> &grid, vector<attribute> &Features,vector<double> Complexity,int flag);
double calError(vector<predict> &Prediction);
vector<predict> predictData(vector<vector<double> > &data,vector<int> index,Node* node,vector<attribute> &Features,int flag);
void predictTree(vector<double> &obs,predict &prediction,Node* node,vector<attribute> &Features,int flag);
void pruneTree(Node* root,double complexity);
pruneBox* pruneUpdate(Node* node,double complexity,vector<pruneBox> &PruneSet);
void printTree(Node* node);
Node* duplicateTree(Node* node);
Node* buildTree(vector<vector<double> >&data,vector<vector<double> >&IndvLoss,vector<double> &grid,vector<int> index, vector<attribute> &Features, Node* node,unsigned depth);
splitFeature fastSplit(vector<vector<double> >&data,vector<vector<double> >&IndvLoss,vector<double> &grid,vector<int> index, vector<attribute> &Features);
splitFeature bestSplit(vector<vector<double> >&data,vector<vector<double> >&IndvLoss,vector<double> &grid,vector<int> index, vector<attribute> &Features);
interval updateInterval(vector<vector<double> >&data,vector<vector<double> >&IndvLoss,vector<double> &grid,vector<int> index, vector<vector<double> >&mosaic, int flag);
interval bestInterval(vector<vector<double> >&data,vector<vector<double> >&IndvLoss,vector<double> &grid,vector<int> index, vector<vector<double> > &mosaic);
double calcLoss(vector<vector<double> >&data,vector<vector<double> >&IndvLoss,vector<int> index,double left,double right);
double sumLoss(vector<vector<double> >&IndvLoss,vector<int> indexIn, vector<int> indexOut);
void updateIndvLoss(vector<vector<double> > *data,vector<vector<double> >&IndvLoss,double &S,double &alpha,int flag);
void printData(vector< vector<double> > table);
void read_csv(const string &inFile,vector<vector<double> >&data,vector<attribute> &Features,int &p,int &n);
void varType(vector<attribute> &Features);
