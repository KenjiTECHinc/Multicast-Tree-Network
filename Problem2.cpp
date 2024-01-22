#include "basicDS.h"

//#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <functional>
#include <map>
#include <algorithm>
/* You can add more functions or variables in each class.
   But you "Shall Not" delete any functions or variables that TAs defined. */

class Problem2 {
public:
    Graph graph;
    Forest gump;
    map<int,int> MTidTraffic;
    map<int,bool> MTactive;
    map<int,Set> MTdestinations;
	Problem2(Graph G);  //constructor
	~Problem2();        //destructor
	bool insert(int id, int s, Set D, int t, Graph &G, Tree &MTid);
	void stop(int id, Graph &G, Forest &MTidForest);
	void rearrange(Graph &G, Forest &MTidForest);
};

Problem2::Problem2(Graph G) {
	/* Write your code here. */
	this->graph = G;
}

Problem2::~Problem2() {
	/* Write your code here. */
    this->MTidTraffic.clear();
	this->graph.V.clear();
	this->graph.E.clear();
	this->gump.trees.clear();
}

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

bool Problem2::insert(int id, int s, Set D, int t, Graph &G, Tree &MTid) {
	/* Store your output graph and multicast tree into G and MTid */
	/* Write your code here. */
	Graph initial_graph;
	initial_graph = this->graph;
    Tree tree;
    MTidTraffic[id] = t;
    MTdestinations[id] = D;
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
    vector<int>::iterator it_int;
    inMST[s] = true;
    tree.V.push_back(s);
    int connected = 0;
    for(it_int = D.destinationVertices.begin(); it_int != D.destinationVertices.end(); ++it_int){
        if(*it_int == s) connected++;
    }
    while(!pq.empty() && (connected < D.size)){
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
            for(it_int = D.destinationVertices.begin(); it_int != D.destinationVertices.end(); ++it_int){
                if(*it_int == v) connected++;
            }
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
    if(connected != D.size){
        //cout <<"connected " << connected << endl;
        //for(auto i:tree.V) cout << i << ' ';
        //cout << endl;
        this->graph = initial_graph;
        tree.E.clear();
        tree.V.clear();
        tree.ct = 0;
        tree.V.push_back(s);
    }
    //optimize path
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
    if(connected != D.size) tree.V.clear();
    MTid = tree;    //output multi-cast tree
    G = this->graph;//output graph
	/* You should return true or false according the insertion result */
	if(connected != D.size){
        MTactive[id] = false;
        return false;
	}
	MTactive[id] = true;
	return true;
}

void Problem2::stop(int id, Graph &G, Forest &MTidForest) {
	/* Store your output graph and multicast tree forest into G and MTidForest
	   Note: Please "only" include mutlicast trees that you added nodes in MTidForest. */
	/* Write your code here. */
    ////////////////////////////////////
	//FIND TREE & ID's TRAFFIC COST
    vector<Tree>::iterator it_Tree;
    for(it_Tree = gump.trees.begin(); it_Tree!=gump.trees.end(); ++it_Tree){
        if((*it_Tree).id == id)break;
    }
    int idTraffic = MTidTraffic.find(id)->second;
    //cout << "this is t: " << idTraffic << endl;
    MTidTraffic.erase(id);
    MTactive.erase(id);
    this->MTdestinations.erase(id);

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
        bool active = MTactive.find((*it_Tree).id)->second;
        if(active == true) continue; //if the tree's edges is already active, then the tree has visited all nodes. Can skip.

        bool flag_Added = false; //flag to notify that this tree has added nodes.
        idTraffic = MTidTraffic.find((*it_Tree).id)->second;
        Set D = MTdestinations.find((*it_Tree).id)->second;
        vector<int>::iterator it_int;

        Graph initial_graph;
        initial_graph = this->graph;
        priority_queue<pair<int,graphEdge>, vector<pair<int,graphEdge>>, PairComparator> pq;

        vector<bool> inMST(this->graph.V.size(), false);
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
        int connected = 0;
        for(it_int = D.destinationVertices.begin(); it_int != D.destinationVertices.end(); ++it_int){
            if(*it_int == (*it_Tree).s) connected++;
        }
        while(!pq.empty() && (connected < D.size)){
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
                for(it_int = D.destinationVertices.begin(); it_int != D.destinationVertices.end(); ++it_int){
                    if(*it_int == v) connected++;
                }
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
        if(connected != D.size){
            this->graph = initial_graph;
            (*it_Tree).E.clear();
            (*it_Tree).V.clear();
            (*it_Tree).ct = 0;
            (*it_Tree).V.push_back((*it_Tree).s);
        }
        else{
            this->MTactive[(*it_Tree).id] = true;
        }
        if((flag_Added == true) && (connected == D.size)){
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

void Problem2::rearrange(Graph &G, Forest &MTidForest) {
	/* Store your output graph and multicast tree forest into G and MTidForest
	   Note: Please include "all" active mutlicast trees in MTidForest. */

	/* Write your code here. */
	vector<graphEdge>::iterator it_graphEdge;
    vector<Tree>::iterator it_Tree;
    Forest ret;
    ret.size = 0;
    for(it_Tree = this->gump.trees.begin(); it_Tree != this->gump.trees.end(); ++it_Tree){
        int idTraffic;
        bool active = MTactive.find((*it_Tree).id)->second;
        if(active == true) continue; //if the tree's edges is already active, then the tree has visited all nodes. Can skip.

        bool flag_Added = false; //flag to notify that this tree has added nodes.
        idTraffic = MTidTraffic.find((*it_Tree).id)->second;
        Set D = MTdestinations.find((*it_Tree).id)->second;
        vector<int>::iterator it_int;

        Graph initial_graph;
        initial_graph = this->graph;
        priority_queue<pair<int,graphEdge>, vector<pair<int,graphEdge>>, PairComparator> pq;

        vector<bool> inMST(this->graph.V.size(), false);
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
        int connected = 0;
        for(it_int = D.destinationVertices.begin(); it_int != D.destinationVertices.end(); ++it_int){
            if(*it_int == (*it_Tree).s) connected++;
        }
        while(!pq.empty() && (connected < D.size)){
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
                for(it_int = D.destinationVertices.begin(); it_int != D.destinationVertices.end(); ++it_int){
                    if(*it_int == v) connected++;
                }
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
        if(connected != D.size){
            this->graph = initial_graph;
            (*it_Tree).E.clear();
            (*it_Tree).V.clear();
            (*it_Tree).ct = 0;
            (*it_Tree).V.push_back((*it_Tree).s);
        }
        else{
            this->MTactive[(*it_Tree).id] = true;
        }
    }
    G = this->graph;
    for(it_Tree = this->gump.trees.begin(); it_Tree!=this->gump.trees.end(); ++it_Tree){
        bool active = this->MTactive.find((*it_Tree).id)->second;
        if(active == true){
            ret.trees.push_back((*it_Tree));
            ret.size++;
        }
    }
    MTidForest = ret;
	return;
}
