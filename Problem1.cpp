#include "basicDS.h"

//#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <functional>
#include <map>
#include <algorithm>
//#include<bits/stdc++.h>
#define INF 99999999
/* You can add more functions or variables in each class.
   But you "Shall Not" delete any functions or variables that TAs defined. */

class Problem1 {
public:
    Graph graph; //duplicate graph
    Graph original; //original graph
    Forest gump; //duplicate forest
    map<int,int> MTidTraffic;
	Problem1(Graph G);  //constructor
	~Problem1();        //destructor
	void insert(int id, int s, Set D, int t, Graph &G, Tree &MTid);
	void stop(int id, Graph &G, Forest &MTidForest);
	void rearrange(Graph &G, Forest &MTidForest);
};

struct PairComparator {
    bool operator()(const std::pair<int, graphEdge>& lhs, const std::pair<int, graphEdge>& rhs) const {
        // Compare based on the first element of the pair
        return lhs.first > rhs.first;
    }
};

struct TreeComparator {
    bool operator()(const Tree &lhs, const Tree &rhs) const{
        // Compare based on the tree's id.
        return lhs.id < rhs.id;
    }
};

Problem1::Problem1(Graph G) {
	/* Write your code here. */
    //initialize params??
    this->graph = G;
    this->original = G;
}

Problem1::~Problem1() {
	/* Write your code here. */
	this->MTidTraffic.clear();
	this->graph.V.clear();
	this->graph.E.clear();
	this->gump.trees.clear();
	//cout << "cleared successfully!" << endl;
}

void Problem1::insert(int id, int s, Set D, int t, Graph &G, Tree &MTid) {
    // id = Multi-cast Tree ID, s = source, D = set of destinations, t = traffic size (bandwidth req.), G = TA's graph, MTid = TA's tree
	/* Store your output graph and multicast tree into G and MTid */
	/* Write your code here. */
	//Create work in our own graph first... then assign them into G and MTid..?
	// no need to use key vector, use the priority queue to store the graph edges and its cost, sorted by min-heap pq
	//Prim's modified
    Tree tree;
    MTidTraffic[id] = t;
    tree.ct = 0;
    tree.id = id;
    tree.s = s;
    priority_queue<pair<int,graphEdge>, vector<pair<int,graphEdge>>, PairComparator> pq; //min-heap with prior queue.
    vector<bool> inMST(D.size, false); //track visited nodes

    vector<graphEdge>::iterator it;
    for(it = this->graph.E.begin(); it!=this->graph.E.end(); ++it){
        if(((*it).vertex[0] != s) && ((*it).vertex[1] != s)) continue;
        else if((((*it).vertex[0] == s) || ((*it).vertex[1] == s)) && (*it).b >= t) pq.push(make_pair((*it).ce, *it));
    }

    inMST[s] = true;
    tree.V.push_back(s);
    while(!pq.empty() && (tree.V.size() < D.size)){
        graphEdge U = pq.top().second;
        int u = (inMST[U.vertex[0]] == true)? U.vertex[0] : U.vertex[1]; //source
        int v = (inMST[U.vertex[0]] == true)? U.vertex[1] : U.vertex[0]; //destination
        pq.pop();

        if((inMST[u] == true) && (inMST[v] == true)){
            continue; //both vertice are visited.
        }
        else if(inMST[v] == false && (U.b >= t)){

            inMST[v] = true; //destination node visited
            tree.V.push_back(v);

            treeEdge edge;
            edge.vertex[0] = u;
            edge.vertex[1] = v;
            tree.E.push_back(edge);
            tree.ct += U.ce * t;
            for(it = this->graph.E.begin(); it != this->graph.E.end(); ++it)
            {
                if(((*it).vertex[0] != v) && ((*it).vertex[1] != v)) continue;
                if((((*it).vertex[0] == u) && (*it).vertex[1] == v) || (((*it).vertex[0] == v) && (*it).vertex[1] == u)){
                    (*it).b -= t; //deduct remaining bandwidth
                }
                if((((*it).vertex[0] == v) || ((*it).vertex[1] == v)) && ((*it).vertex[0] != u && (*it).vertex[1] != u) && ((*it).b >= t)){
                    // if one of the vertex is the current destination.
                    // and the other vertex is not current source.
                    // and remaining bandwidth is >= traffic cost.
                    pq.push(make_pair((*it).ce, *it));
                    //cout << pq.top().first << endl;
                }
            }
        }
    }
    if(!this->gump.trees.empty()){  //keep forest sorted for stop() and rearrange()
        vector<Tree>::iterator it_Tree;
        it_Tree = this->gump.trees.end();
        it_Tree--;
        if((*it_Tree).id > tree.id){
            this->gump.trees.push_back(tree);
            sort(this->gump.trees.begin(), this->gump.trees.end(), TreeComparator());
        }
        else{
            this->gump.trees.push_back(tree);
        }
    }
    else{
        gump.trees.push_back(tree);
    }
    gump.size++;
    MTid = tree;    //output multi-cast tree
    G = this->graph;//output graph
	return;
}

void Problem1::stop(int id, Graph &G, Forest &MTidForest) {
	/* Store your output graph and multicast tree forest into G and MTidForest
	   Note: Please "only" include mutlicast trees that you added nodes in MTidForest. */
	/* Write your code here. */
	//sort the tree vector??? <------------------------- TODO//

	////////////////////////////////////
	//FIND TREE & ID's TRAFFIC COST
    vector<Tree>::iterator it_Tree;
    for(it_Tree = gump.trees.begin(); it_Tree!=gump.trees.end(); ++it_Tree){
        if((*it_Tree).id == id)break;
    }
    int idTraffic = MTidTraffic.find(id)->second;
    //cout << "this is t: " << idTraffic << endl;
    MTidTraffic.erase(id);

    ////////////////////////////////////
    // RETURN TRAFFIC COST
    vector<treeEdge>::iterator it_treeEdge;
    vector<graphEdge>::iterator it_graphEdge;
    int checked = 0;
    for(it_graphEdge = this->graph.E.begin(); it_graphEdge!=this->graph.E.end(); ++it_graphEdge){
        if(checked == (*it_Tree).E.size()){
            //cout << "break stop cycle: " << checked << " E.size: " << (*it_Tree).E.size() << endl;
            break;
        }
        int u = (*it_graphEdge).vertex[0];
        int v = (*it_graphEdge).vertex[1];

        for(it_treeEdge = (*it_Tree).E.begin(); it_treeEdge != (*it_Tree).E.end(); ++it_treeEdge){
            if(((*it_treeEdge).vertex[0] == u && (*it_treeEdge).vertex[1] == v) || ((*it_treeEdge).vertex[0] == v && (*it_treeEdge).vertex[1] == u)){
                (*it_graphEdge).b += idTraffic;
                //cout << (*it_treeEdge).vertex[0] << " " << (*it_treeEdge).vertex[1] << " " << (*it_graphEdge).b << endl;
                checked++;
            }
        }
    }
    //clear memory
    (*it_Tree).V.clear();
    (*it_Tree).E.clear();
    this->gump.trees.erase(it_Tree);
    this->gump.size--;

    ////////////////////////////////////
    // ADD ROUTE TO EXISTING MULTI-CAST TREES
    Forest ret;
    ret.size = 0;
    for(it_Tree = this->gump.trees.begin(); it_Tree != this->gump.trees.end(); ++it_Tree){
        if((*it_Tree).E.size() == (this->graph.V.size()-1)) continue; //if the tree's edges is already equal to V-1, then the tree has visited all nodes. Can skip.

        bool flag_Added = false; //flag to notify that this tree has added nodes.
        idTraffic = MTidTraffic.find((*it_Tree).id)->second;
        vector<int>::iterator it_int;

        priority_queue<pair<int,graphEdge>, vector<pair<int,graphEdge>>, PairComparator> pq;

        vector<bool> inMST(this->graph.V.size(), false); //destination is all nodes anyways
        for(it_int = (*it_Tree).V.begin(); it_int != (*it_Tree).V.end(); ++it_int){
            inMST[(*it_int)] = true;
            //cout << (*it_int) << " is: " << inMST[(*it_int)] << endl;
        }

        for(it_graphEdge = this->graph.E.begin(); it_graphEdge != this->graph.E.end(); ++it_graphEdge){
            if((inMST[(*it_graphEdge).vertex[0]] == true) && inMST[(*it_graphEdge).vertex[1]] == true) continue;
            if(((inMST[(*it_graphEdge).vertex[0]] == true) || inMST[(*it_graphEdge).vertex[1]] == true) && (*it_graphEdge).b >= idTraffic){
                //cout << (*it_graphEdge).vertex[0] << " in: " << inMST[(*it_graphEdge).vertex[0]] << " " << (*it_graphEdge).vertex[1] << " in: " << inMST[(*it_graphEdge).vertex[1]] << endl;
                pq.push(make_pair((*it_graphEdge).ce, (*it_graphEdge)));
            }
        }
        while(!pq.empty() && ((*it_Tree).V.size() < this->graph.V.size())){
            graphEdge U = pq.top().second;
            int u = (inMST[U.vertex[0]] == true)? U.vertex[0] : U.vertex[1]; //source
            int v = (inMST[U.vertex[0]] == true)? U.vertex[1] : U.vertex[0]; //destination
            pq.pop();
            if((inMST[u] == true) && (inMST[v] == true)){
                continue; //both vertices are visited.
            }
            else if(inMST[v] == false && U.b >= idTraffic){
                inMST[v] = true;
                (*it_Tree).V.push_back(v);
                flag_Added = true;

                treeEdge edge;
                edge.vertex[0] = u;
                edge.vertex[1] = v;
                (*it_Tree).E.push_back(edge);
                (*it_Tree).ct += U.ce * idTraffic;
                for(it_graphEdge = this->graph.E.begin(); it_graphEdge != this->graph.E.end(); ++it_graphEdge){
                    if(((*it_graphEdge).vertex[0] != v) && (*it_graphEdge).vertex[1] != v) continue;
                    if((((*it_graphEdge).vertex[0] == u) && (*it_graphEdge).vertex[1] == v) || (((*it_graphEdge).vertex[0] == v) && (*it_graphEdge).vertex[1] == u)){
                        (*it_graphEdge).b -= idTraffic; //deduct remaining bandwidth
                    }
                    if((((*it_graphEdge).vertex[0] == v) || (*it_graphEdge).vertex[1] == v) && (((*it_graphEdge).vertex[0] != u) && ((*it_graphEdge).vertex[1] != u)) && ((*it_graphEdge).b >= idTraffic)){
                        pq.push(make_pair((*it_graphEdge).ce, (*it_graphEdge)));
                    }
                }
            }
        }
        if(flag_Added == true){
            ret.trees.push_back((*it_Tree));
            ret.size++;
        }
    }
    ////////////////////////////////////
    // return output
    G = this->graph; //graph
    MTidForest = ret; //return forest of trees that has added path
	return;
}

void Problem1::rearrange(Graph &G, Forest &MTidForest) {
	/* Store your output graph and multicast tree forest into G and MTidForest
	   Note: Please include "all" active mutlicast trees in MTidForest. */
	/* Write your code here. */
    ////////////////////////////////////
    // RESET GRAPH
    /*
    vector<graphEdge>::iterator it_graphEdge;
    for(it_graphEdge = this->graph.E.begin(); it_graphEdge != this->graph.E.end(); ++it_graphEdge){
        (*it_graphEdge).b = (*it_graphEdge).be;
    }*/
    this->graph = this->original;

    ////////////////////////////////////
    // REARRANGE GRAPH W/ PRIM's
    vector<graphEdge>::iterator it_graphEdge;
    vector<Tree>::iterator it_Tree;
    for(it_Tree = this->gump.trees.begin(); it_Tree != this->gump.trees.end(); ++it_Tree){
        //clear tree
        (*it_Tree).V.clear();
        (*it_Tree).E.clear();
        (*it_Tree).ct = 0;
        int idTraffic = MTidTraffic.find((*it_Tree).id)->second;
        int idSource = (*it_Tree).s;

        priority_queue<pair<int,graphEdge>, vector<pair<int,graphEdge>>, PairComparator> pq;
        vector<bool> inMST(this->graph.V.size(), false);

        for(it_graphEdge = this->graph.E.begin(); it_graphEdge != this->graph.E.end(); ++it_graphEdge){
            if(((*it_graphEdge).vertex[0] != idSource) && (*it_graphEdge).vertex[1] != idSource) continue;
            else if((((*it_graphEdge).vertex[0] == idSource) || ((*it_graphEdge).vertex[1] == idSource)) && (*it_graphEdge).b >= idTraffic) pq.push(make_pair((*it_graphEdge).ce, *it_graphEdge));
        }
        inMST[idSource] = true;
        (*it_Tree).V.push_back(idSource);
        while(!pq.empty()&&((*it_Tree).V.size() < this->graph.V.size())){
            graphEdge U = pq.top().second;
            int u = (inMST[U.vertex[0]] == true)? U.vertex[0] : U.vertex[1];
            int v = (inMST[U.vertex[0]] == true)? U.vertex[1] : U.vertex[0];
            pq.pop();
            if((inMST[u] == true) && (inMST[v] == true)){
                continue;
            }
            else if(inMST[v] == false && (U.b >= idTraffic)){
                inMST[v] = true;
                (*it_Tree).V.push_back(v);

                treeEdge edge;
                edge.vertex[0] = u;
                edge.vertex[1] = v;
                (*it_Tree).E.push_back(edge);
                (*it_Tree).ct += U.ce * idTraffic;
                for(it_graphEdge = this->graph.E.begin(); it_graphEdge != this->graph.E.end(); ++it_graphEdge){
                    if(((*it_graphEdge).vertex[0] != v) && ((*it_graphEdge).vertex[1] != v)) continue;
                    if((((*it_graphEdge).vertex[0] == u) && (*it_graphEdge).vertex[1] == v) || (((*it_graphEdge).vertex[0] == v) && (*it_graphEdge).vertex[1] == u)){
                        (*it_graphEdge).b -= idTraffic; //deduct remaining bandwidth
                    }
                    if((((*it_graphEdge).vertex[0] == v) || ((*it_graphEdge).vertex[1] == v)) && ((*it_graphEdge).vertex[0] != u && (*it_graphEdge).vertex[1] != u) && ((*it_graphEdge).b >= idTraffic)){
                        pq.push(make_pair((*it_graphEdge).ce, *it_graphEdge));
                    }
                }
            }
        }
    }
    ////////////////////////////////////
    // RETURN GRAPH & FOREST
    G = this->graph;
    MTidForest = this->gump;
	return;
}
