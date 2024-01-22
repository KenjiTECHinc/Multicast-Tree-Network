/***********************************************/
//         This .cpp is for Problem2           //
/***********************************************/
#include <iostream>
#include "basicDS.h"
#include "Problem2.cpp"
#include <fstream>
#include <string>
#include <queue>
using namespace std;
void graph_input(Graph& G) {
	int vertex_num, edge_num;
	cin >> vertex_num >> edge_num;

	for (int i = 0; i < vertex_num; i++) {
		G.V.push_back(i);
	}

	for (int i = 0; i < edge_num; i++) {
		graphEdge gE;
		int a, b, c, d;
		cin >> a >> b >> c >> d;
		gE.vertex[0] = a;
		gE.vertex[1] = b;
		gE.b = c;
		gE.be = c;
		gE.ce = d;
		G.E.push_back(gE);
	}
}

void insert_input(int& id, int& s, int& t, Set& D, string input_string) {

	queue<string> q;

	while (1) {
		q.push(input_string.substr(0, input_string.find(" ")));
		input_string = input_string.substr(input_string.find(" ") + 1, input_string.length());

		if (input_string.find(" ") == -1) {
			q.push(input_string);
			break;
		}
	}

	q.pop();
	id = stoi(q.front());
	q.pop();
	s = stoi(q.front());
	q.pop();
	q.pop();

	while (1) {
		string tmp_s = q.front();
		q.pop();
		if (tmp_s == "}") { break; }
		int tmp_i = stoi(tmp_s);
		D.destinationVertices.push_back(tmp_i);
		D.size++;
	}

	t = stoi(q.front());
	q.pop();
}

int stop_input(string input_string) {
	string tmp_s = input_string.substr(input_string.find(" ") + 1, input_string.length());
	return stoi(tmp_s);
}

int main(void) {

	Graph G;
	Tree T;
	Forest F;
	graph_input(G);

	Problem2 P2(G);

	string input_string;
	while (getline(cin, input_string)) {
		if (input_string[0] == 'i') {
			int id, s, t;
			Set D;
            bool take;
			insert_input(id, s, t, D, input_string);
			take = P2.insert(id, s, D, t, G, T);
			if(take == true) cout << "true" <<endl;
			else cout << "false" << endl;
			cout << "cost:" << T.ct << endl;
            for(auto i : T.V){
                cout << "V:" << i << endl;
            }
            for(auto j: D.destinationVertices){
                int flag =0;
                for(auto i : T.V){
                    if(j == i) flag = 1;
                }
                if(flag==0) cout << "Error not traversed to! " << j << endl;
            }
            for(treeEdge edge: T.E){
                cout << "E:" << edge.vertex[0] << ' ' << edge.vertex[1] <<endl;
            }
            for(graphEdge edge: G.E){
                cout << "E :" << edge.vertex[0] << " " << edge.vertex[1] << endl;
                cout << "B : " << edge.b << " Be: " << edge.be << " ce : " << edge.ce <<endl;
            }

		}
		else if (input_string[0] == 's') {
			int id = stop_input(input_string);
			P2.stop(id, G, F);
            cout << "Size after stop: " << F.trees.size() << endl;
            for(Tree pohon: F.trees){
                cout << "Tree ID :" << pohon.id << endl;
                for(treeEdge edge: pohon.E){
                    cout << "E:" << edge.vertex[0] << edge.vertex[1] <<endl;
                }
           }
           for(graphEdge edge: G.E){
                cout << "E :" << edge.vertex[0] << " " << edge.vertex[1] << endl;
                cout << "B : " << edge.b << " Be: " << edge.be << " ce : " << edge.ce <<endl;
           }
		}
		else if (input_string[0] == 'r') {
			P2.rearrange(G, F);
            cout << "after rearrange: " << endl;
            for(Tree pohon: F.trees){
                cout << "Tree ID :" << pohon.id << endl;
                for(treeEdge edge: pohon.E){
                    cout << "E:" << edge.vertex[0] << edge.vertex[1] <<endl;
                }
           }
           for(graphEdge edge: G.E){
                cout << "E :" << edge.vertex[0] << " " << edge.vertex[1] << endl;
                cout << "B : " << edge.b << " Be: " << edge.be << " ce : " << edge.ce <<endl;
           }
		}
	}
	return 0;
}
