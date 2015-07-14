#pragma once
#include "SearchState.h"

SearchState::SearchState()
{
	*this = SearchState::null();
}
SearchState::SearchState(SearchStateResult state)
{
	this->state = state;
}
SearchState::SearchState(SearchStateResult state, std::string foundText)
{
	this->state = state;
	this->foundText = foundText;
}
SearchState::SearchState(SearchStateResult state, std::string foundText, int position)
{
	this->state = state;
	this->foundText = foundText;
	this->position = position;
}

bool SearchState::operator==(SearchState L)
{
	return (state == L.state && foundText == L.foundText);
}
bool SearchState::operator!=(SearchState L)
{
	return !(*this == L);
}

// static region

SearchState SearchState::null() { return SearchState(SearchStateResult::NULL_, "" , -1); }

SearchState SearchState::SearchForward(int searchTextCounter, int textCounter, std::string text, std::string searchText)
{
	while (searchTextCounter < searchText.size())
	{
		if (text[textCounter++] != searchText[searchTextCounter++])	// increment both indices 
			break;
		else
		{
			if (textCounter >= text.size())
			{
				return SearchState(SearchStateResult::SUB_BEGINNING, searchText.substr(0, searchTextCounter), searchTextCounter);	// sub last in the next process
			}
		}
	}

	return SearchState(SearchStateResult::NULL_, "");
}

SearchState SearchState::SearchBackward(int searchTextCounter, int textCounter, std::string text, std::string searchText)
{
	while (searchTextCounter >= 0)
	{
		if (text[textCounter--] != searchText[searchTextCounter--])	// increment both indices 
			break;
		else
		{
			if (textCounter < 0)
			{
				return SearchState(SearchStateResult::SUB_LAST, searchText.substr(searchTextCounter+1, searchText.size()), searchTextCounter);	// sub last in the next process
			}
		}
	}

	return SearchState(SearchStateResult::NULL_, "");
}