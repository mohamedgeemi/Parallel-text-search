#pragma once
#include <vector>
#include <string>

class SearchState;

class SearchNode
{
public :
	int ID;
	std::vector<SearchState> nodeStates;

	SearchNode(int processID);
	SearchNode(SearchState state, int processID); 
	SearchNode(std::vector<SearchState> state, int processID);

	static std::vector<SearchState> Search(std::string text, std::string searchText);

	static int CombineSearchNodes(SearchState state);
private :
	static std::vector<SearchState> FindCommonSubText(std::string text, std::string searchText);
	static bool ParallelCondition(int id, int counter, int end, int remain);
	static std::vector<SearchState> ConcatinateVectors(std::vector<SearchState> a, std::vector<SearchState> b);
};