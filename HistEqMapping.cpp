#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <algorithm>
#include <unordered_map>
#include <functional>
#include<unordered_set>
#include <queue>

using namespace std;

namespace std {
    template<>
    struct hash<set<string>> {
        size_t operator()(const set<string>& s) const {
            size_t hash_val = 0;
            for (const string& str : s) {
                hash_val ^= hash<string>{}(str);
            }
            return hash_val;
        }
    };
}

class WFnet 
{
    public:
        void addPrePlace(const string& transition, const string& place)
        {
            prePlaces[transition].insert(place);
            transitions.insert(transition);
            places.insert(place);
        }
        void addPostPlace(const string& transition, const string& place)
        {
            postPlaces[transition].insert(place);
            transitions.insert(transition);
            places.insert(place);
        }
        void addInitPlace(const string& init)
        {
            initPlace = init;
            places.insert(init);
        }
        vector<tuple<set<string>, set<string>, string>> getReachabilityGraph() const
        {
            reachabilityGraph.clear();
            buildReachabilityGraph();
            return reachabilityGraph;
        }
        set<string> getPlaces() const
        {
            return places;
        }
        string getInitPlace() const 
        {
            return initPlace;
        }

    private:
        set<string> places;
        string initPlace;
        set<string> transitions;
        map<string, set<string>> prePlaces;
        map<string, set<string>> postPlaces;
        mutable vector<tuple<set<string>, set<string>, string>> reachabilityGraph;

        vector<string> enabledTransitions(const set<string>& marking) const 
        {
            vector<string> possibleTransitions;
            for (const auto& entry : prePlaces) {
                bool allPresent = includes(marking.begin(), marking.end(), 
                                entry.second.begin(), entry.second.end());
                if (allPresent) possibleTransitions.push_back(entry.first);
            }
            return possibleTransitions;
        }
        set<string> applyTransition(const set<string>& currentMarking, const string& transition) const
        {
            set<string> sortedMarking(currentMarking.begin(), currentMarking.end()); 
            auto prePlacesIter = prePlaces.find(transition);
            if (prePlacesIter != prePlaces.end()) {
                set<string> sortedPrePlaces = prePlacesIter->second;
                set<string> result1;
                set_difference(currentMarking.begin(), currentMarking.end(),
                            sortedPrePlaces.begin(), sortedPrePlaces.end(),
                            inserter(result1, result1.begin())); 
                auto postPlacesIter = postPlaces.find(transition);
                if (postPlacesIter != postPlaces.end()) {
                    set<string> sortedPostPlaces = postPlacesIter->second;
                    
                    set<string> result2; 
                    set_union(sortedPostPlaces.begin(), sortedPostPlaces.end(),
                                result1.begin(), result1.end(),
                                inserter(result2, result2.begin()));

                    return result2;
                    
                }
                else {
                    cout << "ERROR: Transition doesn't exist in post-places" << endl;
                    return set<string>();
                }
            }
            else {
                cout << "ERROR: Transition doesn't exist in pre-places" << endl;
                return set<string>();
            }
        }
        void printMarking(const set<string>& marking) const
        {
            cout << "{ ";
            for (const auto& place : marking) {
                cout << place << ' ';
            }
            cout << '}';
        }
        void buildReachabilityGraph() const
        {
            set<set<string>> visitedMarkings;
            stack<set<string>> markingStack;

            set<string> initMarking{initPlace};
            markingStack.push(initMarking);

            while (!markingStack.empty()) {
                set<string> currentMarking = markingStack.top();
                markingStack.pop();
                if (visitedMarkings.count(currentMarking)) continue;

                visitedMarkings.insert(currentMarking);

                for (const auto& transition : enabledTransitions(currentMarking)) {
                    set<string> newMarking = applyTransition(currentMarking, transition);
                    markingStack.push(newMarking);
                    reachabilityGraph.push_back(make_tuple(currentMarking, newMarking, transition));
                }
            }
        }
};

class ReachabilityGraph {
    private:
        vector<tuple<set<string>, set<string>, string>> reachabilityGraph;
        set<set<string>> nodes;
        set<string> edges;

        void dfs(const set<string>& node, set<tuple<set<string>, set<string>, string>>& visited, vector<tuple<set<string>, 
        set<string>, string>>& path, vector<vector<tuple<set<string>, set<string>, string>>>& cycles)
        {
            for (const auto& fullEdge : reachabilityGraph) {
                const set<string>& start = get<0>(fullEdge);
                const set<string>& end = get<1>(fullEdge);

                if (start == node) {
                    if (visited.find(fullEdge) != visited.end()) {
                        auto it = find(path.begin(), path.end(), fullEdge);
                        if (it != path.end()) {
                            cycles.push_back(vector<tuple<set<string>, set<string>, string>>(it, path.end()));
                        }
                    }
                    else {
                        visited.insert(fullEdge);
                        path.push_back(fullEdge);
                        dfs(end, visited, path, cycles);
                        path.pop_back();
                        visited.erase(fullEdge);
                    }
                }
            }
        }


    public:
        ReachabilityGraph(const vector<tuple<set<string>, set<string>, string>>& rg)
        {
            for (const auto& triplet : rg) {
                reachabilityGraph.push_back(triplet);
                const set<string>& source = get<0>(triplet);
                const set<string>& destination = get<1>(triplet);
                const string& label = get<2>(triplet);

                nodes.insert(source);
                nodes.insert(destination);

                edges.insert(label);
            }
        }
        set<set<string>> getNodes() const
        {
            return nodes;
        }
        set<string> getEdges() const
        {
            return edges;
        }
        vector<tuple<set<string>, set<string>, string>> getRG() const 
        {
            return reachabilityGraph;
        }

        set<set<tuple<set<string>, set<string>, string>>> findCycles()
        {
            vector<vector<tuple<set<string>, set<string>, string>>> cycles;
            set<tuple<set<string>, set<string>, string>> visited;

            for (const auto& triplet : reachabilityGraph) {
                const set<string>& start = get<0>(triplet);
                const set<string>& end = get<1>(triplet);
                
                vector<tuple<set<string>, set<string>, string>> path;

                dfs(start, visited, path, cycles);
            }
            
            set<set<tuple<set<string>, set<string>, string>>> result;
            for (const auto& cycle : cycles) {
                result.insert(set<tuple<set<string>, set<string>, string>>(cycle.begin(), cycle.end()));
            }

            return result;
        }

        set<string> getEmptyTransitions()
        {
            set<string> emptyEdges;
            for (auto e : edges) {
                if (e.length() >= 5) {
                    if (e.substr(0, 5) == "EMPTY") emptyEdges.insert(e);
                }
            }
            return emptyEdges;
        }

};

vector<set<tuple<set<string>, set<string>, string>>> findSimplePaths(const ReachabilityGraph& oldRG, 
const set<string>& currentNode, const set<string>& endNode, const vector<tuple<set<string>, set<string>, string>>& currentPath, 
vector<vector<tuple<set<string>, set<string>, string>>>& paths, bool start)
{
    if (currentNode == endNode) {
        paths.push_back(currentPath);
        set<set<tuple<set<string>, set<string>, string>>> resultSetSet; 
        for (const auto& path : paths) {
            resultSetSet.insert(set<tuple<set<string>, set<string>, string>>(path.begin(), path.end()));
        }
        vector<set<tuple<set<string>, set<string>, string>>> resultSet(resultSetSet.begin(), resultSetSet.end());
        
        return resultSet;
    }
    for (const auto& edge : oldRG.getRG()) {
        const set<string>& node1 = get<0>(edge);
        const set<string>& node2 = get<1>(edge);
        const string& edgeName = get<2>(edge);
        if (node1 == currentNode) {
            vector<tuple<set<string>, set<string>, string>> updatedPath = currentPath;
            if (find(currentPath.begin(), currentPath.end(), edge) == currentPath.end()) {
                updatedPath.push_back(edge);
                findSimplePaths(oldRG, node2, endNode, updatedPath, paths, false);
            }
        }
    }
    set<set<tuple<set<string>, set<string>, string>>> resultSetSet; 
    for (const auto& path : paths) {
        resultSetSet.insert(set<tuple<set<string>, set<string>, string>>(path.begin(), path.end()));
    }
    vector<set<tuple<set<string>, set<string>, string>>> resultSet(resultSetSet.begin(), resultSetSet.end());
    
    return resultSet;
}

void findComplexPaths(set<set<tuple<set<string>, set<string>, string>>> oldRGcycles, vector<set<tuple<set<string>, set<string>, string>>>& pathList, bool print = false)
{
    if (print) {
        cout << "Initial Path List\n";
        for (int i = 0; i < pathList.size(); i++) {
            set<tuple<set<string>, set<string>, string>> path = pathList[i];
            cout << "[ ";
            for (auto e : path) cout << get<2>(e) << ' ';
            cout << "]\n";
        }
        cout << "\n================\n===================\n\n";
    }
    for (int i = 0; i < pathList.size(); i++) {
        set<tuple<set<string>, set<string>, string>> path = pathList[i];

        if (print) {
            cout << "\n-----------------------------\nIteration " << i << endl;
            for (auto &path : pathList) {
                cout << "[ ";
                for (auto e : path) cout << get<2>(e) << ' ';
                cout << "]\n";
            }
        }
        for (auto edge : path) {
            set<string> startNode = get<0>(edge);
            set<string> endNode = get<1>(edge);
            for (auto cycle : oldRGcycles) {
                bool startNodePresent = false;
                for (auto cycleEdge : cycle) {
                    set<string> cycleEdgeStart = get<0>(cycleEdge);
                    if (cycleEdgeStart == startNode) {
                        startNodePresent = true;
                        if (print) {
                            cout << "Start Node { ";
                            for (auto e : startNode) cout << e << ' ';
                            cout << "} present in cycle [ ";
                            for (auto c : cycle) cout << get<2>(c) << ' ';
                            cout << "]\n";
                        }
                        break;
                    }
                }
                if (startNodePresent) {
                    set<tuple<set<string>, set<string>, string>> setunion;
                    set_union(path.begin(), path.end(),
                            cycle.begin(), cycle.end(),
                            inserter(setunion, setunion.begin()));
                    bool duplicate = false;
                    for (int j = 0; j < pathList.size(); j++) {
                        set<tuple<set<string>, set<string>, string>> tmpPath = pathList[j];
                        if (tmpPath == setunion) {
                            duplicate = true;
                            break;
                        }
                    }
                    if (!duplicate) pathList.push_back(setunion);
                }

                bool endNodePresent = false;
                for (auto cycleEdge : cycle) {
                    set<string> cycleEdgeStart = get<1>(cycleEdge);
                    if (cycleEdgeStart == endNode) {
                        endNodePresent = true;
                        if (print) {
                            cout << "End Node { ";
                            for (auto e : endNode) cout << e << ' ';
                            cout << "} present in cycle [ ";
                            for (auto c : cycle) cout << get<2>(c) << ' ';
                            cout << "]\n";
                        }
                        break;
                    }
                }
                if (endNodePresent) {
                    set<tuple<set<string>, set<string>, string>> setunion;
                    set_union(path.begin(), path.end(),
                            cycle.begin(), cycle.end(),
                            inserter(setunion, setunion.begin()));
                    bool duplicate = false;
                    for (int j = 0; j < pathList.size(); j++) {
                        set<tuple<set<string>, set<string>, string>> tmpPath = pathList[j];
                        if (tmpPath == setunion) {
                            duplicate = true;
                            break;
                        }
                    }
                    if (!duplicate) pathList.push_back(setunion);
                }
            }
        }
    }
};

void getPowerSet(const set<string>& originalSet, vector<set<string>>& result, set<string>& subset, set<string>::iterator iter)
{
    result.push_back(subset);
    for (auto it = iter; it != originalSet.end(); ++it) {
        subset.insert(*it);
        getPowerSet(originalSet, result, subset, next(it));
        subset.erase(*it);
    }
}

unordered_map<set<string>, vector<set<string>>> findEquivalenceMapping(ReachabilityGraph& oldRG, ReachabilityGraph& newRG, const string& oldInitPlace, const string& newInitPlace)
{
    unordered_map<set<string>, vector<set<string>>> mappingTable;
    set<string> oldInitNode = {oldInitPlace};
    set<string> newInitNode = {newInitPlace};
    set<set<tuple<set<string>, set<string>, string>>> oldRGcycles = oldRG.findCycles();
    set<set<tuple<set<string>, set<string>, string>>> newRGcycles = newRG.findCycles();

    cout << "Cycles:\n";
    for (auto c : oldRGcycles) {
        cout << "{ ";
        for (auto tup : c) {
            string e = get<2>(tup);
            cout << e << ' ';
        }
        cout << "}\n";
    }
    // Get the trace transition sets for each node in the new graph
    unordered_map<set<string>, vector<set<string>>> newRGNodesPathsList;
    for (const auto& node : newRG.getNodes()) {
        newRGNodesPathsList[node] = {};
        vector<vector<tuple<set<string>, set<string>, string>>> paths = {};
        vector<set<tuple<set<string>, set<string>, string>>> simplePathList = findSimplePaths(newRG, newInitNode, node, {}, paths, true);
        set<string> tmp = {"P1"};
        if (node == tmp) {
            cout << "\n\n=========================\nsimple paths for P4\n";
            for (auto &simplePath : simplePathList) {
                cout << "[ ";
                for (auto &e : simplePath) cout << get<2>(e) << ' ';
                cout << "]\n";
            }
            cout << "\n\n\nSTARTED\n\n\n";
            findComplexPaths(newRGcycles, simplePathList, true);
            cout << "\n\n\nENDED\n\n\n\n";
        }
        else findComplexPaths(newRGcycles, simplePathList);
        set<string> newRGemptyEdges = newRG.getEmptyTransitions();

        for (const auto& path : simplePathList) {
            set<string> pathEdges;
            for (auto p : path) {
                pathEdges.insert(get<2>(p));
            }
            set<string> nonEmptyPathEdges;
            set_difference(pathEdges.begin(), pathEdges.end(), newRGemptyEdges.begin(), newRGemptyEdges.end(), inserter(nonEmptyPathEdges, nonEmptyPathEdges.begin()));
            newRGNodesPathsList[node].push_back(nonEmptyPathEdges);
        }

    }

    for (const auto& node : oldRG.getNodes()) {
        cout << "Paths for { ";
        for (auto nn : node) cout << nn << ' ';
        cout << "}:\n";
        
        vector<vector<tuple<set<string>, set<string>, string>>> paths = {};
        vector<set<tuple<set<string>, set<string>, string>>> simplePathList = findSimplePaths(oldRG, oldInitNode, node, {}, paths, true);
        
       findComplexPaths(oldRGcycles, simplePathList);
       for (auto path : simplePathList) {
            cout << "[ ";
            for (auto p : path) cout << get<2>(p) << ' ';
            cout << "]\n"; 
        }
        cout << "\n\n";
        set<string> oldRGemptyEdges = oldRG.getEmptyTransitions();
        vector<set<string>> mappingTableNode;
        for (const auto& path : simplePathList) {
            set<string> pathEdges;
            for (auto p : path) {
                pathEdges.insert(get<2>(p));
            }
            set<string> nonEmptyPathEdges;

            set_difference(pathEdges.begin(), pathEdges.end(), oldRGemptyEdges.begin(), oldRGemptyEdges.end(), inserter(nonEmptyPathEdges, nonEmptyPathEdges.begin()));

            for (const auto& newRGTTSpair : newRGNodesPathsList) {
                set<string> newRGnode = newRGTTSpair.first;
                vector<set<string>> traceTransitionSets = newRGTTSpair.second;
                for (const auto& traceTransitionSet : traceTransitionSets) {
                    if (traceTransitionSet == nonEmptyPathEdges) {
                        mappingTableNode.push_back(newRGnode);
                        break;
                    }
                }
            }
        }
        mappingTable[node] = {};
        for (const auto& mtne : mappingTableNode) {
            if (find(mappingTable[node].begin(), mappingTable[node].end(), mtne) == mappingTable[node].end()) {
                mappingTable[node].push_back(mtne);
            }
        }
    }
    return mappingTable;
}



int main (int argc, char * argv[])
{
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <oldWFnetFile> <newWFnetFile>" << endl;
        return 1;
    }
    ifstream file1(argv[1]);
    ifstream file2(argv[2]);
    if (!file1.is_open()) {
        cerr << "Error opening file: " << argv[1] << endl;
        return 1;
    }
    if (!file2.is_open()) {
        cerr << "Error opening file: " << argv[2] << endl;
        return 1;
    }
    WFnet oldWFNet, newWFNet;

    string line;
    while (getline(file1, line)) {
        istringstream iss(line);
        string type, transition;
        iss >> type >> transition;
        if (type == "INIT") {
            oldWFNet.addInitPlace(transition);
            break;
        }
        string place;
        while (iss >> place) {
            if (type == "PRE") {
                oldWFNet.addPrePlace(transition, place);
            } 
            else if (type == "POST") {
                oldWFNet.addPostPlace(transition, place);
            }
            else {
                cerr << "Invalid input" << endl;
                return 1;
            }            
        }
    }
    file1.close();
    while (getline(file2, line)) {
        istringstream iss(line);
        string type, transition;
        iss >> type >> transition;
        if (type == "INIT") {
            newWFNet.addInitPlace(transition);
            break;
        }
        string place;
        while (iss >> place) {
            if (type == "PRE") {
                newWFNet.addPrePlace(transition, place);
            } 
            else if (type == "POST") {
                newWFNet.addPostPlace(transition, place);
            }
            else {
                cerr << "Invalid input" << endl;
                return 1;
            }            
        }
    }
    file2.close();

    ReachabilityGraph oldReachabilityGraph(oldWFNet.getReachabilityGraph()), newReachabilityGraph(newWFNet.getReachabilityGraph());

    set<set<string>> nodes = oldReachabilityGraph.getNodes();

    unordered_map<set<string>, vector<set<string>>> eqMap = findEquivalenceMapping(oldReachabilityGraph, newReachabilityGraph, oldWFNet.getInitPlace(), newWFNet.getInitPlace());

    for (const auto& pair : eqMap) {
        cout << "Old Node: { ";
        for (const auto& element : pair.first) {
            cout << element << ' ';
        }
        cout << "}\tEquivalent Nodes in new Graph: [ ";
        for (const auto& setElement : pair.second) {
            cout << "{ ";
            for (const auto& innerElement : setElement) {
                cout << innerElement << ' ';
            }
            cout << "}\t";
        }
        cout << "]\n";
    }
 
}