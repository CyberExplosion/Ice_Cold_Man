#include <iostream>
#include <queue>
#include <list>
#include <vector>
#include <future>
#include <atomic>
#include <condition_variable>
#include <execution>
#include <algorithm>
#include <cmath>
#include <map>
using namespace	std;

const int IFN = 9999;

//Let's say we have a case of a map with lot's of vertices's and few edges
//[A][B][][D]
//[E][][G][H]
//[J][K][L][M]

//Find the path from J to D shortest

vector<pair<int, int>> emptyIce_vec;

mutex locker;

//Need to convert the emptyIce_vec into a graph
//Adjacent graph
struct Verticle {
	int distance;
	pair<int, int >location;
	Verticle(int d, pair<int, int>l): distance(d), location(std::move(l)) {};
};

vector<list<Verticle>> graph;

//Create edges between vertices
void createEdge() {
	vector<Verticle> neighbor_vec;
	for (auto& verticle : graph) {
			for_each(begin(graph), end(graph),
				[&](list<Verticle> neighborList) {	//If there are vertices that are adjacent then create an edge for them
					if ((neighborList.front().location.first == verticle.front().location.first + 4) || (neighborList.front().location.first == verticle.front().location.first - 4) || (neighborList.front().location.second == verticle.front().location.second + 4) || (neighborList.front().location.second == verticle.front().location.second - 4))
						neighbor_vec.push_back(neighborList.front());	//Push the neighbor into the vector
				});

			for (auto& neighbor : neighbor_vec) {
				verticle.push_back(neighbor);	//Add the neighbor into your list of neigbors
			}
			neighbor_vec.clear();	//Prepare for another neighbor_vec
	}
}

//Only add one more vertice into the already existing graph. Going to need to update the dist value every time you call this
void addNewVertice(pair<int, int> locale) {
	Verticle vert(IFN, std::move(locale));
	list<Verticle> vertList;
	vertList.push_front(std::move(vert));

	//Add edges to the new vertices
	vector<Verticle> neighbor_vec;
	for_each(begin(graph), end(graph),
		[&neighbor_vec, vertList](list<Verticle> neighborList) {
			if ((neighborList.front().location.first == vertList.front().location.first + 4) || (neighborList.front().location.first == vertList.front().location.first - 4) || (neighborList.front().location.second == vertList.front().location.second + 4) || (neighborList.front().location.second == vertList.front().location.second - 4))
				neighbor_vec.push_back(vertList.front());	//Find all possible edges
		});
	for (auto& neighbor : neighbor_vec) {	//Add edges here
		vertList.push_back(neighbor);
	}

	graph.push_back(std::move(vertList));	//Put the new vertice with edges into the graph
}

//Populate the graph with empty ice_vector
void populateGraph() {
	for (auto& val : emptyIce_vec){
		Verticle vert(IFN, val);	//Set the distance of the graph at infinity
		list<Verticle> vertice;
		vertice.push_back(vert);

		graph.push_back(vertice);	//Push the vertice into the graph
	}
	createEdge();
}

//Hash functor for unordered_map
struct pairHash {
	std::size_t operator()(pair<int, int> val) const {	//Function need to be const for unordered map to reconized
		return static_cast<std::size_t>(sizeof(int) + sizeof(int));	//Return the size of a pair of integers
	}
};

//Assigning field value from source - AKA distance from a source
unordered_map<pair<int, int>, int, pairHash> distValueGenerate(pair<int, int> src) {

	auto start = find_if(execution::par, begin(graph), end(graph),	//Start will return the iterator to the starting Vertices List
		[&src](list<Verticle> vert) {
			return vert.front().location == src;	//Find the correct verticle to start from
		});

	//Create a map to store the distance needs to arrive at a particular vertice
	unordered_map<pair<int, int>, int, pairHash> distTravel;

	scoped_lock<mutex> betterLock(locker);	//All of next step is critical

	if (start != end(graph)) {
		//Create a queue for breadth first search. It will hold the neighbors and vertices
		queue<list<Verticle>> que;

		//Set the start node distance and preliminary stuff
		start->front().distance = 0;
		que.push(*start);	//Push the starting Vertice as the first victim
		distTravel[start->front().location] = 0;	//Set the distance need to travel to starting location be 0

		//Going through all the neighbor vertices
		while (!que.empty()) {
			//Pop the vertices out to work with it
			auto neighbors = que.front();	//If just start the loop, ** neighbors == start **
			que.pop();

			//distTravel[neighbors.front().location] = distTravel[start->front().location] + 1;	//The distance to the neighbor vertices is the distance at beginning plus 1

			//Get all adjacent vertices, if it's not visited then mark it's visited then push it to the queue
			for (auto& next_neighbor : neighbors) {	
				if (distTravel.count(next_neighbor.location) == 0) {	//If we haven't visited this neighbor yet --> The distance to the neighbor not exist

					next_neighbor.distance = distTravel[neighbors.front().location] + 1;	//Distance from the first neighbor to the next one is an increase of 1
					distTravel[next_neighbor.location] = next_neighbor.distance;	//Update the travel distance to "this" neighbor

					//Find the neighbor's list of neighbor to continue expanding
					auto next = find_if(execution::par, begin(graph), end(graph), [next_neighbor](list<Verticle> val) {return val.front().location == next_neighbor.location; });
					if (next != end(graph))
						que.push(*next);
				}
			}
		}
	}
	return distTravel;
}


int main() {
	vector<pair<int, int>> temp;

	auto time1 = chrono::high_resolution_clock::now();
	for (int i = 64; i >= 4; i -= 4) {
		temp.push_back(make_pair(30, i));
	}
	emptyIce_vec = std::move(temp);	//Fill the ice vector
	populateGraph();
	addNewVertice(make_pair(30, 68));
	auto fut = async(launch::async, distValueGenerate, std::move(make_pair(30, 64)));
	auto result = fut.get();
	auto time2 = chrono::high_resolution_clock::now();
	chrono::duration<double, std::milli> timer = time2 - time1;
	cout << "The time using async: " << timer.count() << "\n";

	for (auto& val : result) {
		cout << "Location: x = " << val.first.first << ", y = " << val.first.second << ". Weight: " << val.second << "\n";
	}

	auto timer1 = chrono::high_resolution_clock::now();
	for (int i = 64; i >= 4; i -= 4) {
		temp.push_back(make_pair(30, i));
	}
	emptyIce_vec = std::move(temp);	//Fill the ice vector
	populateGraph();
	addNewVertice(make_pair(30, 68));
	auto result2 = distValueGenerate(make_pair(30, 64));
	auto timer2 = chrono::high_resolution_clock::now();
	chrono::duration<double, std::milli> rr = timer2 - timer1;
	cout << "The time using normal coding: " << rr.count() << "\n";

	for (auto& val : result2) {
		cout << "Location: x = " << val.first.first << ", y = " << val.first.second << ". Weight: " << val.second << "\n";
	}
}