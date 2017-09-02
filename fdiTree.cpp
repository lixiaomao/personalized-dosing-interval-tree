//============================================================================
// Name        : fdi_tree.cpp
// Author      : littleraccoon
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include "declare.h"

unsigned minSize;
unsigned maxDepth;
unsigned nfolds;
double alpha;
double S;
unsigned sided;
//
int main(int argc, char* argv[]) {


	string trainFile(argv[1]);
	string testFile(argv[2]);
	S=atof(argv[4]);
	alpha=atof(argv[5]);
	sided=atoi(argv[6]);
	maxDepth=atoi(argv[7]);
	minSize=atoi(argv[8]);
	nfolds=atoi(argv[9]);

	//vector<double> Complexity={0,0.25,0.5,0.75,1,1.25,1.5,1.75,2,2.25,2.5,2.75,3,3.25,3.5,4,4.5,5,6,7,8,9,10,12.5,15};
	vector<double> Complexity;
	for (int i=0;i<50;i++){
		Complexity.push_back(i*0.2);
	}

	vector < vector<double> > train;
	vector < vector<double> > test;

	string singleline;
	int p=0,n=0,p1=0,n1=0;

	vector<attribute> Features,Features1;

	read_csv(trainFile,train,Features,p,n);
	read_csv(testFile,test,Features1,p1,n1);

	varType(Features);

	for (unsigned j=0;j<Features.size();j++){
		cout<< Features[j].name<<" isCategorical "<<Features[j].discrete<< " levels ";
		for (unsigned i=0;i<Features[j].levels.size();i++){
			cout<<Features[j].levels[i]<<" ";
		}
		cout<<endl;
	}


	cout<<"train length "<<train.size()<<endl;
	cout<<"test length "<<test.size()<<endl;


	int ngrid=100;
	vector<double> grid;
	double maxA=*max_element(Features[2].value.begin(),Features[2].value.end());
	double minA=*min_element(Features[2].value.begin(),Features[2].value.end());
	double step=(maxA-minA)/ngrid;
	for (int i=0;i<(ngrid-2);i++) grid.push_back(minA+i*step);
	grid.push_back(999);
	grid.insert(grid.begin(),-999);

	vector<int> index;
	for (int i=0;i<n;i++){
		index.push_back(i);
	}

	random_device rd;
	mt19937 g(rd());
	shuffle(index.begin(), index.end(), g);
	//copy(index.begin(), index.end(), ostream_iterator<int>(cout, " "));
	vector<vector<double> > IndvLoss;
	updateIndvLoss(&train,IndvLoss,S,alpha,0);
	vector<vector<double> > mosaic(grid.size(), vector<double>(grid.size(),0));

    IndvLoss.clear();
    updateIndvLoss(&train,IndvLoss,S,alpha,0);
    Node *root=new Node;
    //buildTree(train,IndvLoss,grid,index,Features,root,0);
    //printTree(root);

	IndvLoss.clear();
	updateIndvLoss(&train,IndvLoss,S,alpha,0);
	Model* PDI=cvTree(train,test,IndvLoss,grid,Features,Complexity,0);
	printTree((*PDI).node);





	return 0;
}

/*
 *
 *
 *	interval ivtmp=updateInterval(train,IndvLoss,grid,index,mosaic,1);
    cout<<" the interval "<<ivtmp.left<<" "<<ivtmp.right<<endl;
    ivtmp=bestInterval(train,IndvLoss,grid,index,mosaic);
    cout<<" the interval "<<ivtmp.left<<" "<<ivtmp.right<<endl;
    cout << "sdfjldsjfsdfdsfdsl"<<endl;;
 *
 *
 *
 * vector<vector<double> > IndvLoss;
	updateIndvLoss(&train,IndvLoss,S,alpha,0);
	Model* PDI=cvTree(train,test,IndvLoss,grid,Features,Complexity,0);
	printTree((*PDI).node);

	IndvLoss.clear();
	updateIndvLoss(&train,IndvLoss,S,alpha,1);
	Model* EDI=cvTree(train,test,IndvLoss,grid,Features,Complexity,1);
	printTree((*EDI).node);

	ofstream out;
	out.open(argv[3]);
	out << PDI->cvLoss <<" " << PDI->testLoss <<" " << PDI->complexity << " " <<EDI->cvLoss<< " " <<EDI->testLoss<< " " <<EDI->complexity<<"\n";
	//out << PDI->cvLoss <<" " << PDI->testLoss <<" " << PDI->complexity << "\n";
	out.close();
 *	//interval iv;
	//vector<vector<double> > mosaic;
	//for (int i=0;i<ngrid;i++){
	//	vector<double> vtmp;
	//	for (int j=0;j<ngrid;j++) vtmp.push_back(0);
	//	mosaic.push_back(vtmp);
	//}

	//iv=bestInterval(train,IndvLoss,grid,index,mosaic);
	//vector<int> aa={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21};
	//int flag=-1;
	//iv=updateInterval(train,IndvLoss,grid,aa,mosaic,flag);
	//splitFeature toy=bestSplit(train,IndvLoss,grid,index,Features);
	//splitFeature toy=fastSplit(train,IndvLoss,grid,index,Features);
	//cout << left<< "   "<< right << "   "<< min << endl;
	 *
 *Node root;
	buildTree(train,IndvLoss,grid,index,Features,&root,0);


	cout<<endl;
	Node* rootNew=duplicateTree(&root);
	pruneBox* box=pruneTree(rootNew,0.001);
	printTree(rootNew);
	vector<predict> newPrediction=predictData(train,index,rootNew);
	cout << calError(newPrediction) <<endl;
	newPrediction=predictData(test,index,rootNew);
	cout << calError(newPrediction) <<endl;

	cout<<endl;
	rootNew=duplicateTree(&root);
	box=pruneTree(rootNew,0.1);
	printTree(rootNew);
	newPrediction=predictData(train,index,rootNew);
	cout << calError(newPrediction) <<endl;
	newPrediction=predictData(test,index,rootNew);
		cout << calError(newPrediction) <<endl;

	cout<<endl;
	rootNew=duplicateTree(&root);
	box=pruneTree(rootNew,0.5);
	printTree(rootNew);
	newPrediction=predictData(train,index,rootNew);
	cout << calError(newPrediction) <<endl;
	newPrediction=predictData(test,index,rootNew);
		cout << calError(newPrediction) <<endl;

	cout<<endl;
	rootNew=duplicateTree(&root);
	box=pruneTree(rootNew,100);
	printTree(rootNew);
	newPrediction=predictData(train,index,rootNew);
	cout << calError(newPrediction) <<endl;
	newPrediction=predictData(test,index,rootNew);
		cout << calError(newPrediction) <<endl;

	 for (int i=0;i<p;i++){
		 for (int j=0;j<p;j++){
			 cout << train[i][j] << endl;
		 }
	 }

	  */
	/*
	for (int j=0;j<p+3;j++){
				 cout << Features[j].name << endl;
	}
	cout << p << endl;
	cout << n << endl;

	void printData(vector< vector<double> > table)

	*/
