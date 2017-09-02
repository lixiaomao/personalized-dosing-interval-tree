/*
 * functions.cpp
 *
 *  Created on: Mar 31, 2017
 *      Author: aa
 */
#include "declare.h"

struct splitCmp{
	bool operator()(const splitData& first,const splitData& second)const{
		if(first.value < second.value || (first.value == second.value && first.index < second.index))
			return true;
		else
			return false;
	}
};
struct sortLevels{
	bool operator()(const double first,const double second)const{
		if(first < second-0.00001)
			return true;
		else
			return false;
	}
};

Model* cvTree(vector<vector<double> >&data,vector<vector<double> >&test,vector<vector<double> >&IndvLoss,vector<double> &grid,vector<attribute> &Features,vector<double> Complexity,int flag){
	vector<int> indexTRAIN,indexTEST;
	for (unsigned i=0;i<data.size();i++) indexTRAIN.push_back(i);
	for (unsigned i=0;i<test.size();i++) indexTEST.push_back(i);

	vector<double> cplxError(Complexity.size(),0.0);
	int foldSize=data.size()/nfolds;
	for (unsigned i=0;i<nfolds;i++){
		cout<<"fold "<<i<<endl;
		vector<int> trainIndex;
		vector<int> testIndex;
		for (unsigned j=0;j<data.size();j++){
			if ((j>=foldSize*i)&&(j<foldSize*(i+1))){
				testIndex.push_back(j);
			} else{
				trainIndex.push_back(j);
			}
		}
		Node* newNode=new Node;
		buildTree(data,IndvLoss,grid,trainIndex,Features,newNode,0);
		printTree(newNode);
		for (unsigned k=0;k<Complexity.size();k++){
			Node* dupNew=duplicateTree(newNode);
			pruneTree(dupNew,Complexity[k]);
			vector<predict> newPrediction=predictData(data,testIndex,dupNew,Features,flag);
			cplxError[k]+=(calError(newPrediction)/Complexity.size());
		}
	}
	for (unsigned k=0;k<Complexity.size();k++) cout<<cplxError[k]<<" ";
	cout<<endl;
	Model* best=new Model;
	(*best).cvLoss=9999999999;
	for (unsigned k=0;k<Complexity.size();k++){
		if (cplxError[k]<((*best).cvLoss-0.0000001)){
			(*best).cvLoss=cplxError[k];
			(*best).complexity=Complexity[k];
		}
	}
	Node* newNode=new Node;
	buildTree(data,IndvLoss,grid,indexTRAIN,Features,newNode,0);
	pruneTree(newNode,(*best).complexity);
	vector<predict> newPrediction=predictData(test,indexTEST,newNode,Features,flag);
	(*best).testLoss=calError(newPrediction);
	(*best).node=newNode;
	cout<<endl;
	cout<<endl;
	cout<<"FINAL TREE"<<endl;
	printTree(newNode);
	cout <<"best cvLoss "<<(*best).cvLoss <<" best testLoss "<<(*best).testLoss<<" complexity selected"<<(*best).complexity<<endl;
	return best;
}
double calError(vector<predict> &Prediction){
	double tmp=0;
	for (auto it=Prediction.begin();it!=Prediction.end();it++){
		tmp+=(*it).loss;
	}
	tmp=tmp/Prediction.size();
	return tmp;
}
vector<predict> predictData(vector<vector<double> > &data,vector<int> index,Node* node,vector<attribute> &Features,int flag){
	vector<predict> Prediction;
	for (unsigned i=0;i<index.size();i++){
		predict *tmp=new predict;
		predictTree(data[index[i]],(*tmp),node,Features,flag);
		Prediction.push_back((*tmp));
		delete(tmp);
	}
	return(Prediction);
}
void predictTree(vector<double> &obs,predict &prediction,Node* node,vector<attribute> &Features,int flag){
	if (node->isLeaf==true){
		prediction.treatment=obs[2];
		prediction.outcome=obs[1];
		prediction.left=node->iv.left;
		prediction.right=node->iv.right;
		if ((prediction.treatment<prediction.right)&&(prediction.left<prediction.treatment)){
			prediction.loss=(flag==0)? ((obs[1]<S)*alpha):(((obs[1]<S)? (S-obs[1]):0)*alpha);
		} else{
			prediction.loss=(flag==0)? ((obs[1]>S)*(1-alpha)):(((obs[1]>S)? (obs[1]-S):0)*(1-alpha));
		}
		return;
	} else {
		if (Features[node->spf.vindex].discrete){
			if (obs[node->spf.vindex]==node->spf.split_value){
				predictTree(obs,prediction,node->Children[0],Features,flag);
			} else {
				predictTree(obs,prediction,node->Children[1],Features,flag);
			}
		} else{
			if (obs[node->spf.vindex]<node->spf.split_value){
				predictTree(obs,prediction,node->Children[0],Features,flag);
			} else {
				predictTree(obs,prediction,node->Children[1],Features,flag);
			}
		}

	}
	return;
}
void pruneTree(Node* root,double complexity){
	bool flag=false;
	cout<< "prune the tree "<< complexity<<endl;
	while (!flag){
		vector<pruneBox> PruneSet;
		pruneUpdate(root,complexity,PruneSet);
		flag=true;
		for (unsigned i=2;i<10;i++){
			if (flag){
				double weakest=0;
				int target=-1;
				for (unsigned j=0;j<PruneSet.size();j++){
					if ((PruneSet[j].score>weakest)&&(PruneSet[j].size==i)){
						weakest=PruneSet[j].score;
						target=j;
					}
				}
				if (target>=0){
					PruneSet[target].node->isLeaf=true;
					PruneSet[target].node->Children.clear();
					flag=false;
				}
			}
		}
	}



}
pruneBox* pruneUpdate(Node* node,double complexity,vector<pruneBox> &PruneSet){
	pruneBox* self=new pruneBox;
	if (node->isLeaf==true){
		self->size=1;
		self->reduction=0;
		return self;
	}
	pruneBox* child1=pruneUpdate(node->Children[0],complexity,PruneSet);
	pruneBox* child2=pruneUpdate(node->Children[1],complexity,PruneSet);
	self->node=node;
    self->size=child1->size+child2->size;
    self->reduction=child1->reduction+child2->reduction;
    self->reduction+=(node->spf.after-node->spf.before);
    self->score=(self->reduction+complexity*self->size);
    PruneSet.push_back(*self);
    //cout<<" Depth " <<node->depth <<" #n "<<node->index.size()<<" size "<<self->size<<" reduction "<<self->reduction<< " score "<<(self->reduction+complexity*self->size)<<endl;

    return self;
}
void printTree(Node* node){
	if (node->isLeaf==true){
		for (unsigned k=0;k<node->depth;k++) cout<<"--";
		cout << "leafNode depth  " << node->depth;
		for (unsigned k=0;k<30-node->depth*2;k++) cout<<" ";
		cout<<(node->iv).left <<"  "<< (node->iv).right <<"  Loss  "<< (node->iv).loss <<"  #n " <<node->index.size()<<endl;
	} else {
		for (unsigned k=0;k<node->depth;k++) cout<<"==";
		cout << "internalNode "<< (node->spf).name <<" " <<(node->spf).split_value <<"  depth  " <<node->depth;
		for (unsigned k=0;k<25-node->depth*2;k++) cout<<" ";
		cout<<(node->iv).left <<"  "<< (node->iv).right <<"  Loss  "<< (node->iv).loss <<"  #n " <<node->index.size()<<endl;
		for (unsigned i=0;i<2;i++){
			printTree(node->Children[i]);
		}
	}
}
Node* duplicateTree(Node* node){
	Node* newNode=new Node;
	newNode->depth=node->depth;
	newNode->index=node->index;
	newNode->iv=node->iv;
	newNode->spf=node->spf;
	newNode->isLeaf=node->isLeaf;
	if (newNode->isLeaf!=1){
		for (unsigned i=0;i<2;i++){
			(newNode->Children).push_back(duplicateTree(node->Children[i]));
		}
	}
	return newNode;
}
Node* buildTree(vector<vector<double> >&data,vector<vector<double> >&IndvLoss,vector<double> &grid,vector<int> index, vector<attribute> &Features, Node* node,unsigned depth){

	cout<< "build a tree"<< depth<< " size "<<index.size()<<endl;
	node->depth=depth;
	node->index=index;
	vector<vector<double> > mosaic(grid.size(), vector<double>(grid.size(),0));
	interval ivtmp=updateInterval(data,IndvLoss,grid,index,mosaic,1);
	node->iv=ivtmp;
	//cout<<" the interval "<<ivtmp.left<<" "<<ivtmp.right<<endl;
	if ((index.size()<minSize)||(depth>maxDepth)){
		node->isLeaf=true;
		for (unsigned k=0;k<depth;k++) cout<<"--";
		cout << "leafNode depth  " << node->depth;
		for (unsigned k=0;k<30-depth*2;k++) cout<<" ";
		cout<<(node->iv).left <<"  "<< (node->iv).right <<"  Loss  "<< (node->iv).loss <<"  #n " <<index.size()<<endl;
		return node;
	}
	splitFeature spf=fastSplit(data,IndvLoss,grid,index,Features);
	if (spf.name=="NULL"){
		node->isLeaf=true;
		for (unsigned k=0;k<depth;k++) cout<<"--";
		cout << "leafNode depth  " << node->depth;
		for (unsigned k=0;k<30-depth*2;k++) cout<<" ";
	    cout<<(node->iv).left <<"  "<< (node->iv).right <<"  Loss  "<< (node->iv).loss <<"  #n " <<index.size()<<endl;
		return node;
	}
	node->isLeaf=false;
	node->spf=spf;
	for (unsigned k=0;k<node->depth;k++)  cout<<"++";
	cout << "internalNode "<< (node->spf).name <<" " <<(node->spf).split_value <<"  depth  " <<node->depth;
	for (unsigned k=0;k<25-node->depth*2;k++) cout<<" ";
	cout<<(node->iv).left <<"  "<< (node->iv).right <<"  Loss  "<< (node->iv).loss <<"  #n " <<index.size()<<endl;

	Node* newNodeL=new Node;
	Node* newNodeR=new Node;
	node->Children.push_back(buildTree(data,IndvLoss,grid,(node->spf).leftindex,Features,newNodeL,depth+1));
	node->Children.push_back(buildTree(data,IndvLoss,grid,(node->spf).rightindex,Features,newNodeR,depth+1));
	cout<<"buildTree finished"<<endl;
	return node;
}
splitFeature fastSplit(vector<vector<double> >&data,vector<vector<double> >&IndvLoss,vector<double> &grid,vector<int> index, vector<attribute> &Features){
	unsigned oindex; // index of the observation where the optimal variable got split
	splitFeature split;
	interval ivtmp;
	int flagL=1,flagR=-1;
	vector<vector<double> > mosaic(grid.size(), vector<double>(grid.size(),0));
	ivtmp=updateInterval(data,IndvLoss,grid,index,mosaic,1);
	split.before=ivtmp.loss;
	split.after=split.before;
	for (unsigned j=3;j<Features.size();j++){
		//vector<vector<double> > mosaicL(grid.size(), vector<double>(grid.size(),0));
		//vector<vector<double> > mosaicR(grid.size(), vector<double>(grid.size(),0));
		//cout << "feature "<< j-2<< " name "<< Features[j].name<<endl;
		vector<splitData> SplitData;
		for (auto it=index.begin();it!=index.end();it++){
			splitData tmp;
			tmp.value=data[*it][j];
			tmp.index=*it;
			SplitData.push_back(tmp);
		}
		vector<int> leftIndex,rightIndex;
		if (Features[j].discrete){
			for (unsigned k=0;k<Features[j].levels.size();k++){
				vector<vector<double> > mosaicL(grid.size(), vector<double>(grid.size(),0));
				vector<vector<double> > mosaicR(grid.size(), vector<double>(grid.size(),0));
				for (unsigned i=0;i<SplitData.size();i++){
					if (SplitData[i].value==Features[j].levels[k]){
						leftIndex.push_back(SplitData[i].index);
					} else{
						rightIndex.push_back(SplitData[i].index);
					}
				}
				interval ivtmp1,ivtmp2;
				ivtmp1=updateInterval(data,IndvLoss,grid,leftIndex,mosaicL,1);
				ivtmp2=updateInterval(data,IndvLoss,grid,rightIndex,mosaicR,1);
				double total=(ivtmp1.loss+ivtmp2.loss);
				if (total<split.after-0.00000001){
					split.after=total;
					split.name=Features[j].name;
					split.split_value=Features[j].levels[k];
					split.vindex=j;
				}
				rightIndex.clear();
				leftIndex.clear();
			}
		} else {
			sort(SplitData.begin(),SplitData.end(),splitCmp());
			for (unsigned i=0;i<SplitData.size();i++) rightIndex.push_back(SplitData[i].index);
			vector<vector<double> > mosaicL(grid.size(), vector<double>(grid.size(),0));
			vector<vector<double> > mosaicR(grid.size(), vector<double>(grid.size(),0));
			updateInterval(data,IndvLoss,grid,rightIndex,mosaicR,1);
			for (unsigned i=0;i<SplitData.size()-1;i++){
				vector<int> tmp;
				leftIndex.push_back(SplitData[i].index);
				rightIndex.erase(rightIndex.begin());

				tmp.push_back(SplitData[i].index);

				interval ivtmp1,ivtmp2;
				ivtmp1=updateInterval(data,IndvLoss,grid,tmp,mosaicL,flagL);
				ivtmp2=updateInterval(data,IndvLoss,grid,tmp,mosaicR,flagR);
				double total=(ivtmp1.loss+ivtmp2.loss);
				//cout << (SplitData[i].value+SplitData[i+1].value)/2 << "   "<<  split.name<< endl;
				//cout <<"first "<<ivtmp1.left<< "  "<< ivtmp1.right<<"  "<<ivtmp1.loss<<"  "<<leftIndex.size()<<endl;
				//cout <<"second "<<ivtmp2.left<< "  "<< ivtmp2.right<<"  "<<ivtmp2.loss<<"  "<<rightIndex.size()<<endl;
				if ((i<minSize)||((SplitData.size()-i-1)<minSize)){
					continue;
				}
				if (total<split.after-0.00000001){
					split.after=total;
					split.name=Features[j].name;
					split.split_value=(SplitData[i].value+SplitData[i+1].value)/2;
					split.vindex=j;
					oindex=i;
					//cout << split.split_value << "  BBBBBIIINNNNGGGGOOO  "<<  split.name<< endl;
					//cout <<"first "<<ivtmp1.left<< "  "<< ivtmp1.right<<"  "<<ivtmp1.loss<<"  "<<i+1<<endl;
					//cout <<"second "<<ivtmp2.left<< "  "<< ivtmp2.right<<"  "<<ivtmp2.loss<<"  "<<SplitData.size()-i-1<<endl;
					//cout << "total " <<ivtmp1.loss+ivtmp2.loss << endl;
				}
			}
		}

	}
	if (split.after>split.before-0.00000001){
		split.name="NULL";
		split.vindex=0;
		return split;
	}
	if (Features[split.vindex].discrete){
		for (unsigned i=0;i<index.size();i++){
			if (data[index[i]][split.vindex]==split.split_value){
				split.leftindex.push_back(data[index[i]][split.vindex]);
			} else {
				split.rightindex.push_back(data[index[i]][split.vindex]);
			}
		}
	} else {
		vector<splitData> SplitData;
		for (auto it=index.begin();it!=index.end();it++){
			splitData tmp;
			tmp.value=data[*it][split.vindex];
			tmp.index=*it;
			SplitData.push_back(tmp);
		}
		sort(SplitData.begin(),SplitData.end(),splitCmp());
		for (unsigned i=0;i<=oindex;i++) split.leftindex.push_back(SplitData[i].index);
		for (unsigned i=oindex+1;i<SplitData.size();i++) split.rightindex.push_back(SplitData[i].index);
	}


	return split;
}

splitFeature bestSplit(vector<vector<double> >&data,vector<vector<double> >&IndvLoss,vector<double> &grid,vector<int> index, vector<attribute> &Features){
	splitFeature split;
	interval ivtmp;
	vector<vector<double> > mosaic(grid.size(), vector<double>(grid.size(),0));
	ivtmp=bestInterval(data,IndvLoss,grid,index,mosaic);
	split.before=ivtmp.loss;
	split.after=split.before;
	for (unsigned j=3;j<8;j++){
		cout << "feature "<< j-2<< "name "<< Features[j].name<<endl;
		vector<splitData> SplitData;
		for (auto it=index.begin();it!=index.end();it++){
			splitData tmp;
			tmp.value=data[*it][j];
			tmp.index=*it;
			SplitData.push_back(tmp);
		}
		sort(SplitData.begin(),SplitData.end(),splitCmp());
		vector<int> leftIndex,rightIndex;
		for (unsigned i=0;i<SplitData.size();i++){
			rightIndex.push_back(SplitData[i].index);
		}
		for (unsigned i=0;i<SplitData.size()-1;i++){
			leftIndex.push_back(SplitData[i].index);
			rightIndex.erase(rightIndex.begin());

			interval ivtmp1,ivtmp2;
			ivtmp1=bestInterval(data,IndvLoss,grid,leftIndex,mosaic);
			ivtmp2=bestInterval(data,IndvLoss,grid,rightIndex,mosaic);
			double total=(ivtmp1.loss+ivtmp2.loss);
			//cout << (SplitData[i].value+SplitData[i+1].value)/2 << "   "<<  split.name<< endl;
			//cout <<"first "<<ivtmp1.left<< "  "<< ivtmp1.right<<"  "<<ivtmp1.loss<<"  "<<leftIndex.size()<<endl;
			//cout <<"second "<<ivtmp2.left<< "  "<< ivtmp2.right<<"  "<<ivtmp2.loss<<"  "<<rightIndex.size()<<endl;
			if (total<split.after-0.00000001){
				split.after=total;
				split.name=Features[j].name;
				split.split_value=(SplitData[i].value+SplitData[i+1].value)/2;
				cout << split.split_value << "  BBBBBIIINNNNGGGGOOO  "<<  split.name<< endl;
				cout <<"first "<<ivtmp1.left<< "  "<< ivtmp1.right<<"  "<<ivtmp1.loss<<"  "<<leftIndex.size()<<endl;
				cout <<"second "<<ivtmp2.left<< "  "<< ivtmp2.right<<"  "<<ivtmp2.loss<<"  "<<rightIndex.size()<<endl;
				cout << "total " <<ivtmp1.loss+ivtmp2.loss << endl;
			}

		}
	}
	return split;
}

interval updateInterval(vector<vector<double> >&data,vector<vector<double> >&IndvLoss,vector<double> &grid,vector<int> index, vector<vector<double> >&mosaic, int flag){
	double tmp=9999;
	interval iv;
	iv.loss=99999;
	iv.left=*grid.begin();
	iv.right=*grid.end();
	double df=iv.right-iv.left;
	//cout<< iv.left<<" "<< iv.right<<" "<< iv.loss<<" "<<endl;
	for (unsigned i=0;i<grid.size()-1;i++){
		for (unsigned j=i+1;j<grid.size();j++){
			for (auto it=index.begin();it!=index.end();it++){
				if ((data[*it][2]>((sided==2)? grid[0]:grid[i]))&&(data[*it][2]<((sided==1)? grid[grid.size()-1]:grid[j]))){
					mosaic[i][j]+=IndvLoss[*it][1]*flag;
				} else{
					mosaic[i][j]+=IndvLoss[*it][0]*flag;
				}
			}
			tmp=mosaic[i][j];
			if (tmp<(iv.loss-0.000000000000001)){
				iv.loss=tmp;
				iv.left=((sided==2)? grid[0]:grid[i]);
				iv.right=((sided==1)? grid[grid.size()-1]:grid[j]);
				df=iv.right-iv.left;
				//cout<< iv.left<<" "<< iv.right<<" "<< iv.loss<<" "<<endl;
			} else if ((tmp<(iv.loss+0.000000000000001))&&(df<(((sided==2)? grid[0]:grid[i])-((sided==1)? grid[grid.size()-1]:grid[j])-0.00000001))){
				iv.loss=tmp;
				iv.left=((sided==2)? grid[0]:grid[i]);
				iv.right=((sided==1)? grid[grid.size()-1]:grid[j]);
				df=iv.right-iv.left;
			}
		}
	}
	//cout<< iv.left<<" "<< iv.right<<" "<< iv.loss<<" "<<endl;
	return iv;
}

interval bestInterval(vector<vector<double> >&data,vector<vector<double> >&IndvLoss,vector<double> &grid,vector<int> index, vector<vector<double> >&mosaic){
	double tmp=9999;
	interval iv;
	iv.loss=99999;
	iv.left=*grid.begin();
	iv.right=*grid.end();
	double df=iv.right-iv.left;
	for (unsigned i=0;i<grid.size()-1;i++){
		for (unsigned j=i+1;j<grid.size();j++){
			tmp=calcLoss(data,IndvLoss,index,grid[i],grid[j]);
			mosaic[i][j]=mosaic[i][j]+tmp;
			if (tmp<(iv.loss-0.000000000000001)){
				iv.loss=tmp;
				iv.left=grid[i];
				iv.right=grid[j];
				df=iv.right-iv.left;
			} else if ((tmp<(iv.loss+0.000000000000001))&&(df<(grid[i]-grid[j]-0.00000001))){
				iv.loss=tmp;
				iv.left=grid[i];
				iv.right=grid[j];
				df=iv.right-iv.left;
			}
		}
	}
	return iv;
}

double calcLoss(vector<vector<double> >&data,vector<vector<double> >&IndvLoss,vector<int> index,double left,double right){
	double tmp=0;
	int count=0;
	for (auto it=index.begin();it!=index.end();++it,++count){
		if ((data[*it][2]>left)&&(data[*it][2]<right)){
			tmp+=IndvLoss[*it][1];
		} else{
			tmp+=IndvLoss[*it][0];
		}
	}
	//tmp=tmp/count;
	return tmp;
}


double sumLoss(vector<vector<double> >&IndvLoss,vector<int> indexIn, vector<int> indexOut){
	double tmp=0;
	for (auto it=indexIn.begin();it!=indexIn.end();++it){
		tmp+=IndvLoss[*it][1];
	}
	for (auto it=indexOut.begin();it!=indexOut.end();++it){
		tmp+=IndvLoss[*it][0];
	}
	return tmp;
}





void updateIndvLoss(vector<vector<double> > *data,vector<vector<double> >&IndvLoss,double &S,double &alpha,int flag){
	if (flag==0){ //PDI
		for (unsigned i=0;i<(*data).size();i++){
			vector<double> tp;
			tp.push_back(((*data)[i][1]>S)*(1-alpha)/(*data)[i][0]);
			tp.push_back(((*data)[i][1]<S)*alpha/(*data)[i][0]);
			IndvLoss.push_back(tp);
		}
	} else if (flag==1){ // EDI
		for (unsigned i=0;i<(*data).size();i++){
			vector<double> tp;
			tp.push_back((((*data)[i][1]>S)? ((*data)[i][1]-S): 0)*(1-alpha)/(*data)[i][0]);
			tp.push_back(((((*data)[i][1]<S)? (S-(*data)[i][1]) : 0))*alpha/(*data)[i][0]);
			IndvLoss.push_back(tp);
		}
	}

}

void printData(vector< vector<double> > table){
	for( unsigned i = 0; i < table.size(); i ++){
		for(unsigned j = 0; j < table[i].size(); j ++)
			cout << table[i][j] << " ";
		cout << endl;
	}
}

void read_csv(const string &inFile,vector<vector<double> >&data,vector<attribute> &Features,int &p,int &n){

	Features.clear();
	string singleline;
	double value;

	ifstream inData;
	inData.open(inFile.c_str());
	if (!inData.is_open() ) {
	      cout <<" Failed to open " <<inFile<< endl;
	}

	getline(inData,singleline);
	while (singleline.length()>0){
	  attribute feature;
	  int pos=singleline.find_first_of(",");
	  if (pos==-1) pos=singleline.length();
	  feature.name=singleline.substr(0,pos);
	  singleline.erase(0,pos+1);
	  Features.push_back(feature);
	  p++;
	}
	p=p-3;

	while (getline(inData,singleline)){
	  vector<double> individual;
	  for (int j=0;j<p+3;j++){
	    int pos=singleline.find_first_of(",");
	    istringstream(singleline.substr(0,pos))>>value;
	    Features[j].value.push_back(value);
	    individual.push_back(value);
	    singleline.erase(0,pos+1);
	    //cout <<value<<" "<<endl;
	  }
	  data.push_back(individual);
	  n++;
	}
}

void varType(vector<attribute> &Features){
	for (unsigned i=0;i<Features.size();i++){
		vector<double > record;
		for (unsigned j=0;j<Features[i].value.size();j++){
			if (record.size()==0){
				record.push_back(Features[i].value[j]);
			} else {
				int flag=0;
				for (unsigned k=0;k<record.size();k++){
					if (abs(record[k]-Features[i].value[j])<0.0001) flag=1;
				}
				if (flag==0){
					record.push_back(Features[i].value[j]);
				}
			}
			if (record.size()>10) break;
		}
		if (record.size()<10){
			Features[i].discrete=true;
			for (unsigned k=0;k<record.size();k++) Features[i].levels.push_back(record[k]);
			sort(Features[i].levels.begin(),Features[i].levels.end(),sortLevels());
		}
	}
}
/* outcome and treatment definition
	int pos=singleline.find_first_of(",");
	outcome.name=singleline.substr(0,pos);
	singleline.erase(0,pos+1);

	pos=singleline.find_first_of(",");
	treatment.name=singleline.substr(0,pos);
	singleline.erase(0,pos+1);


   /// later
    * istringstream(singleline.substr(0,pos))>>value;
	  outcome.value.push_back(value);
	  individual.push_back(value);
	  singleline.erase(0,pos+1);
	  pos=singleline.find_first_of(",");
	  istringstream(singleline.substr(0,pos))>>value;
	  treatment.value.push_back(value);
	  individual.push_back(value);
	  singleline.erase(0,pos+1);
	*/
