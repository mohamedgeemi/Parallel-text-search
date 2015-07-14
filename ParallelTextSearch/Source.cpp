#pragma once 
#include <Windows.h>
#include <omp.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>
#include "mpi.h"
#include <ctime>

#include "SearchNode.h"
#include "SearchState.h"

#define BEGIN_LAST_KEYWORD 5
#define LAST_BEGIN_KEYWORD 6
#define ALL_KEYWORD 1
#define NOT_FOUND 0
#define MIDDLE_OF_KEYWORD 2
#define SPLITTER char(1)

#pragma warning(disable : 4996)

using namespace std;

MPI_Status* recieveStatus = new MPI_Status();

char* StringToCharArray(string s)
{
	char* str = new char [s.size() + 1];
	for (int i = 0; i < s.size(); ++i)
		*(str + i) = s[i];
	*(str + s.size()) = '\0';
	return str;
}
string GetText(string file)
{
	string text = "", tmp;
	ifstream fileStream(file);

	if (fileStream.is_open())
	{
		while (fileStream.good())
		{
			getline(fileStream, tmp);
			text = text + tmp;
		}
	}
	else
		cout << "Not Open\n";
	return text;
}
void ShowInfo(string text , int segment)
{
	cout << "text size = " << text.size() << endl; 
	cout << text << endl;

	cout << "number of threads: " << omp_get_num_threads() << endl;
	cout << "segment size :" << segment << endl << endl;
}
void ShowMessage(int id, SearchNode foundState)
{
	printf("from thread number (%d) text found %d \n", id, foundState.nodeStates.size());
	for (int i = 0; i < foundState.nodeStates.size(); ++i)
	{
		printf("%s ,", StringToCharArray(foundState.nodeStates[i].foundText));
		// why the hell it's not working!!! ~_~
	}
	printf("\n");
}

string MessageFormat(SearchNode foundState) {
	string message = "";

	//printf("foundState.nodeStates size = %d\n", foundState.nodeStates.size());


	if (foundState.nodeStates.size() == 1) {
		if (foundState.nodeStates[0].state == SearchStateResult::NULL_) {
			message += SearchStateResult::NULL_ + '0';
		}
		else if (foundState.nodeStates[0].state == SearchStateResult::FOUND) {
			message += SearchStateResult::FOUND + '0';
			message += SPLITTER + foundState.nodeStates[0].foundText;
			message += SPLITTER + to_string(foundState.nodeStates[0].position);
		} 
		else if (foundState.nodeStates[0].state == SearchStateResult::SUB_MIDDLE) {
			message += SearchStateResult::SUB_MIDDLE + '0';
			message += SPLITTER + foundState.nodeStates[0].foundText;
			//message += " " + foundState.nodeStates[0].position;
		}

		else if (foundState.nodeStates[0].state == SearchStateResult::SUB_BEGINNING) {
			message += SearchStateResult::SUB_BEGINNING + '0';
			message += SPLITTER + foundState.nodeStates[0].foundText;
			//message += " " + foundState.nodeStates[0].position;
		}

		else if (foundState.nodeStates[0].state == SearchStateResult::SUB_LAST) {
			message += SearchStateResult::SUB_LAST + '0';
			message += SPLITTER + foundState.nodeStates[0].foundText;
			//message += " " + foundState.nodeStates[0].position;
		}
	} 
	else if (foundState.nodeStates.size() == 2) {
		if (foundState.nodeStates[0].state == SearchStateResult::SUB_BEGINNING && 
			foundState.nodeStates[1].state == SearchStateResult::SUB_LAST) {
				message += BEGIN_LAST_KEYWORD + '0';
				message += SPLITTER + foundState.nodeStates[0].foundText;
				//message += " " + foundState.nodeStates[0].position;
				message += SPLITTER + foundState.nodeStates[1].foundText;
				//message += " " + foundState.nodeStates[1].position;
		} 
		else if (foundState.nodeStates[0].state == SearchStateResult::SUB_LAST &&
			foundState.nodeStates[1].state == SearchStateResult::SUB_BEGINNING) {
				message += LAST_BEGIN_KEYWORD + '0';
				message += SPLITTER + foundState.nodeStates[0].foundText;
				//message += " " + foundState.nodeStates[0].position;
				message += SPLITTER + foundState.nodeStates[1].foundText;
				//message += " " + foundState.nodeStates[1].position;
		}

		else if (foundState.nodeStates[0].state == SearchStateResult::SUB_MIDDLE) {
				message += foundState.nodeStates[1].state + '0';
				message += SPLITTER + foundState.nodeStates[1].foundText;
				//message += " " + foundState.nodeStates[1].position;
		}
		else if (foundState.nodeStates[1].state == SearchStateResult::SUB_MIDDLE) {
				message += foundState.nodeStates[0].state + '0';
				message += SPLITTER + foundState.nodeStates[0].foundText;
				//message += " " + foundState.nodeStates[0].position;
		}
	}

	return message;
}

vector<string> split(string message, char splitter) {
	vector<string> tokens;
	string temp = "";

	for (int i = 0; i < message.length(); i++) {
		if (message[i] == splitter) {
			tokens.push_back(temp);
			temp = "";

			continue;
		}

		temp += message[i];
	}

	if (temp != "")
		tokens.push_back(temp);

	return tokens;
}
int getOriginalIndex(int textSize, int numOfProcess, int currentProcess, int localIndex)
{
	int position = (currentProcess - 1)*(textSize / numOfProcess) + localIndex;
	if (currentProcess > (textSize % numOfProcess))
		position += (textSize % numOfProcess);
	else
		position += currentProcess - 1;
	
	return position;
}
// Share the data with the slaves
void shareData(int processRank, string text, string targetKeyword)
{
	int subTextSize = text.size() / processRank;
	int textPerProcessesRemaining = text.size() % processRank;
	int offset = 0;
	char* textCharsArray = StringToCharArray(text);
	char* targetKeywordArray = StringToCharArray(targetKeyword);
	int targetKeywordSize = targetKeyword.size();

	//printf("subtextSize = %d\n", subTextSize);


	for (int i = 0; i < processRank; i++) {
		int textSize = subTextSize;

		if (textPerProcessesRemaining) {
			textPerProcessesRemaining--;
			textSize++;
			MPI_Send(&textSize, 1, MPI_INT, i + 1, i + 1, MPI_COMM_WORLD);
			MPI_Send(textCharsArray + offset, textSize, MPI_CHAR, i + 1, i + 1, MPI_COMM_WORLD);
		}
		else {
			MPI_Send(&textSize, 1, MPI_INT, i + 1, i + 1, MPI_COMM_WORLD);
			MPI_Send(textCharsArray + offset, textSize, MPI_CHAR, i + 1, i + 1, MPI_COMM_WORLD);
		}

		MPI_Send(&targetKeywordSize, 1, MPI_INT, i + 1, i + 1, MPI_COMM_WORLD);
		//printf("target size sended\n");

		MPI_Send(targetKeywordArray, targetKeywordSize, MPI_CHAR, i + 1, i + 1, MPI_COMM_WORLD);
		//printf("targetArray sended\n");
		offset += textSize;
	}
}

// Distrbute the paragraph into the slaves processes 
void master(int numOfProcess, string text, string targetKeyword) {
	
	shareData(numOfProcess, text, targetKeyword); // Share to processes  ( slaves )

	vector<int> keywordIndices;
	char* processMessage;
	int processMessageLength = 0;
	bool hasBegin = false;
	string concatenatedKeyword = "";
	int position = -1;

	for (int slave = 1; slave <= numOfProcess; slave++) {
		MPI_Recv(&processMessageLength, 1, MPI_INT, slave, MPI_ANY_TAG, MPI_COMM_WORLD, recieveStatus);

		processMessage = new char[processMessageLength];

		MPI_Recv(processMessage, processMessageLength, MPI_CHAR, slave, MPI_ANY_TAG, MPI_COMM_WORLD, recieveStatus);

		string processMessageString = string(processMessage, (processMessage + processMessageLength));
		vector<string> messageTokens = split(processMessageString, SPLITTER);


		if (processMessage[0] == SearchStateResult::NULL_ + '0')
			continue;

		else if (processMessage[0] == SearchStateResult::FOUND + '0') {

			position = getOriginalIndex(text.size(), numOfProcess, slave, atoi(StringToCharArray(messageTokens[2])));

			printf("Keyword is found at process %d, position %d\n", slave, position);

			hasBegin = false;
			concatenatedKeyword = "";
			position = -1;
			//break;
		}
		else if (processMessage[0] == SearchStateResult::SUB_BEGINNING + '0') {
			/*if (!hasBegin) {
				concatenatedKeyword = messageTokens[1];
				hasBegin = true;
			} else {
				concatenatedKeyword = messageTokens[1];
			}*/
			concatenatedKeyword = messageTokens[1];
			hasBegin = true;

			int localIndex = (text.size() / numOfProcess) - messageTokens[1].size();
			if (slave <= text.size() % numOfProcess)
				localIndex++;
			position = getOriginalIndex(text.size(), numOfProcess, slave, localIndex);
		}
		else if (processMessage[0] == SearchStateResult::SUB_LAST + '0') {
			string lastKeyword = messageTokens[1];

			if (hasBegin)
			{
				concatenatedKeyword += lastKeyword;
				if (concatenatedKeyword.find(targetKeyword) != std::string::npos) {
					printf("Keyword is found at process %d, position %d\n", slave, position);
				}

				concatenatedKeyword = "";
				hasBegin = false;
			}
		}
		else if (processMessage[0] == SearchStateResult::SUB_MIDDLE + '0')
		{
			string subKeyword = messageTokens[1];

			if (hasBegin)
			{
				concatenatedKeyword += subKeyword;
			}
		}
		else if (processMessage[0] == BEGIN_LAST_KEYWORD + '0')
		{
			cout << slave << endl;
			string beginKeyword = messageTokens[1];
			string lastKeyword = messageTokens[2];

			if (hasBegin)
			{
				concatenatedKeyword += lastKeyword;
				if (concatenatedKeyword.find(targetKeyword) != std::string::npos) {
					printf("Keyword is found at process %d, position %d\n", slave, position);
				}

				concatenatedKeyword = beginKeyword;
			}
			else
			{
				concatenatedKeyword = beginKeyword;
				hasBegin = true;
			}
		}
		else if (processMessage[0] == LAST_BEGIN_KEYWORD + '0')
		{
			cout << slave << endl;
			string beginKeyword = messageTokens[2];
			string lastKeyword = messageTokens[1];

			if (hasBegin)
			{
				concatenatedKeyword += lastKeyword;
				if (concatenatedKeyword.find(targetKeyword) != std::string::npos) {
					printf("Keyword is found at process %d, position %d\n", slave, position);
				}

				concatenatedKeyword = beginKeyword;
			}
			else
			{
				concatenatedKeyword = beginKeyword;
				hasBegin = true;
			}
		}
	}

}

// search on the keyword
void slave(int slaveRank) {
	bool found = false;
	int subTextSize = 0;
	char* localTargetKeyword;
	int targetKeywordSize = 0;


	//... Get the subtext size
	MPI_Recv(&subTextSize, 1, MPI_INT, 0, slaveRank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	//printf("subText size recived = %d\n", subTextSize);

	char* subTextArray = new char[subTextSize];

	//... Get the subText elements
	MPI_Recv(subTextArray, subTextSize, MPI_CHAR, 0, slaveRank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	//printf("subText = %s\n", subTextArray);

	printf("Process %d recieved %d char(s)\n", slaveRank, subTextSize);
	//cout << "Text : \"" << string(subTextArray, (subTextArray + subTextSize)) << "\"" << endl;

	//... Get the target size
	MPI_Recv(&targetKeywordSize, 1, MPI_INT, 0, slaveRank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	localTargetKeyword = new char[targetKeywordSize];
	//... Get the target keyword
	MPI_Recv(localTargetKeyword, targetKeywordSize, MPI_CHAR, 0, slaveRank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	string subTextString = string(subTextArray, (subTextArray + subTextSize));
	string targetKeyword = string(localTargetKeyword, (localTargetKeyword + targetKeywordSize));
	SearchNode foundState = SearchNode(slaveRank);

	//cout << "subText String = " << subTextString << endl;
	//cout << "subText String size = " << subTextString.size() << endl;

	foundState = SearchNode(SearchNode::Search(subTextString, targetKeyword), slaveRank); // The state of the messege 

	string message = MessageFormat(foundState);
	cout << "slave " << slaveRank << ", message : " << "\"" << message << "\"" << endl;
	
	int messageLength = message.size();
	char* messageArray = StringToCharArray(message);

	MPI_Send(&messageLength, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	MPI_Send(messageArray, messageLength, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
}


int main(int argc, char* argv[])
{
	int rank = 0;
	int numberOfProcesses = 0;

	// intialize MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);

	omp_set_num_threads(omp_get_max_threads());

	// ID of the process 
	if (rank == 0) 
	{
		string text = GetText(argv[1]); // get the text
		
		string targetKeyword;
		cout << "\nType the search word: ";
		getline(cin, targetKeyword);
		

		clock_t begin = clock(); // calculate the performance 

		// Master work .. send to all processes segemnt of text
		master(numberOfProcesses - 1, text, targetKeyword);

		clock_t end = clock();
		double elapsedTime = double(end - begin) / CLOCKS_PER_SEC;

		printf("\nElapsed Time: %lf seconds", elapsedTime);
	}
	else 
	{
		slave(rank);
	}
	
	MPI_Finalize();

	return 0;
}