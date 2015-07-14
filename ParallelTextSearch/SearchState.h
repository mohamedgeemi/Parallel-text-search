#include <string>
#include "SearchStateResult.h"
class SearchState
{
public :
	SearchStateResult state;
	std::string foundText;
	int position;
	
	SearchState();
	SearchState(SearchStateResult state);
	SearchState(SearchStateResult state, std::string foundText);
	SearchState(SearchStateResult state, std::string foundText , int position);

	bool operator==(SearchState L);
	bool operator!=(SearchState L);

	// static region

	static SearchState null();

	static SearchState SearchForward(int searchTextCounter, int textCounter, std::string text, std::string searchText);

	static SearchState SearchBackward(int searchTextCounter, int textCounter, std::string text, std::string searchText);
	
	
};
