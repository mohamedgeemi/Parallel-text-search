#pragma once
#include "SearchNode.h"
#include "SearchState.h"
#include <omp.h>

SearchNode::SearchNode(int processID)
{
	ID = processID;
}
SearchNode::SearchNode(std::vector<SearchState> state, int processID)
{
	nodeStates = state; 
	ID = processID; 
}
SearchNode::SearchNode(SearchState state, int processID)
{
	nodeStates.push_back( state );
	ID = processID;
}
bool SearchNode::ParallelCondition(int id, int counter, int end, int remain)
{
	return id == omp_get_max_threads() - 1 ?
		counter < end + remain :
		counter < end; 
}
std::vector<SearchState> SearchNode::ConcatinateVectors(std::vector<SearchState> a, std::vector<SearchState> b)
{
	std::vector<SearchState> concat = a;
	for (int i = 0; i < b.size(); ++i)
		concat.push_back(b[i]);
	return concat;
}
std::vector<SearchState> SearchNode::FindCommonSubText(std::string text, std::string searchText)
{
	std::vector<SearchState> ret; 
	omp_set_num_threads(omp_get_max_threads());

	int chunkSize = text.size() / omp_get_max_threads(),
		chunkSizeRemain = text.size() % omp_get_max_threads();

	int id;
#pragma omp parallel private(id)
	{
		id = omp_get_thread_num();
		int start = id * chunkSize,	// 0 
			end = (id + 1) * chunkSize;

		for (int textCounterForward = start, textCounterBackward = end-1;
			ParallelCondition(id, textCounterForward, end, chunkSizeRemain);
			++textCounterForward, --textCounterBackward)	// forward search
		{
			SearchState tmpResult;

			// parallelize these both
			int searchTextCounterForward = 0;
			tmpResult = SearchState::SearchForward(searchTextCounterForward, textCounterForward, text, searchText);
			if (tmpResult != SearchState::null())
				ret.push_back(tmpResult);

			int searchTextCounterBackward = searchText.size() - 1;
			tmpResult = SearchState::SearchBackward(searchTextCounterBackward, textCounterBackward, text, searchText);
			if (tmpResult != SearchState::null())
				ret.push_back(tmpResult);
		}

	}

	return ret; 
}
std::vector<SearchState> SearchNode::Search(std::string text, std::string searchText)
{
	std::vector<SearchState> ret = std::vector<SearchState>();
	if (text.find(searchText) != std::string::npos)
	{
		SearchState tmp = SearchState(SearchStateResult::FOUND, searchText, text.find(searchText));
		ret.push_back(tmp);
		return ret;
	}

	if (searchText.find(text) != std::string::npos)	// if substring of the keyword found in the all text process
		ret.push_back(SearchState(SearchStateResult::SUB_MIDDLE, text));
	
	std::vector<SearchState> retCommon = FindCommonSubText(text, searchText);
	ret = ConcatinateVectors( ret , retCommon); // ret sent by reference 

	return ret;
}

int SearchNode::CombineSearchNodes(SearchState state)
{
	return 0;
}