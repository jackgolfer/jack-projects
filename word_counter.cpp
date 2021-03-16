/* Name: Jack Splaine 
Date: 12/5/19
Section: 7
Assignment: #6
Due Date: 12/6/19

All work below was performed by Jack Splaine */

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <list>

using namespace std;
void insert(string s, int pos, map<string,pair<int, int>> & m);
bool compare_pair(const pair<string,pair<int, int>> & first, const pair<string,pair<int, int>> & second);
bool compare_char(const pair<char, int> & first, const pair<char, int> & second);


int main()
{
	//creates all maps, lists, and variables used throughout the greater portion of the code
	int charArray[99] = {};
	map <string,pair<int, int>> wordList, numList;
	string str;
	char tempChar;
	int tempInt;
	bool curWord = 0, curNum = 0;
	list<pair<string,pair<int, int>>> stringList;
	list<pair<char, int> > charList;
	int position = 0;

	//loop that reads input, ends at end of file
	while (cin.eof() == 0)
	{
		//takes char from input and stores it
		tempChar = cin.get();
		//adds to the counter for that character
		tempInt = tempChar;
		//increments the array that keeps track of character occurence
		if(tempChar == '\n')
			charArray[95]++;
		else if(tempChar == '\t')
			charArray[96]++;
		else if(tempChar == '\v')
			charArray[97]++;
		else if(tempChar == '\r')
			charArray[98]++;
		else
			charArray[tempInt-32]++;


		if ((tempChar >= 65 && tempChar <= 90) || (tempChar >= 97 && tempChar <= 122)) //char input is a letter
		{
			//makes letter lowercase
			tempChar = tolower(tempChar);
			//word already being made, adds to word
			if(curWord == 1)
			{
				str += tempChar;
			}
			//number was being made, inserts number and starts new word
			else if(curNum == 1)
			{
				curNum = 0;
				curWord = 1;
				insert(str, position++, numList);
				str = tempChar;
			}
			//neither word nor number being made, starts new word
			else if(curNum == 0 && curWord == 0)
			{
				curWord = 1;
				str = tempChar;
			}
		}
		else if(tempChar >= 48 && tempChar <= 57) //char input is a number
		{
			//number already being made, adds to number
			if(curNum == 1)
			{
				str += tempChar;
			}
			//word was being made, inserts word and starts new number
			else if(curWord == 1)
			{
				curNum = 1;
				curWord = 0;
				insert(str, position++, wordList);
				str = tempChar;
			}
			//neither word nor number being made, starts new number
			else if(curNum == 0 && curWord == 0)
			{
				curNum = 1;
				str = tempChar;
			}
		}
		else //char input is neither a letter or number
		{
			if(curWord == 1) //word was being made, inserts word
				insert(str, position++, wordList);
			else if(curNum == 1) //number was being made, inserts number
				insert(str, position++, numList);
			curWord = 0;
			curNum = 0;
		}
	}
	//finds total number of unique characters used
	int charNum = 0;
	for (int i=0; i<99;i++)
		if (charArray[i] > 0)
			charNum++;

	pair<char, int> tempPair;
	//main loop that traverses the character array, pushes character into top ten list if it fits
	for (int i=0;i<95;i++)
	{
		if(charArray[i] > 0)
		{
			tempPair.first = i+32;
			tempPair.second = charArray[i];
			charList.push_back(tempPair);
			//functor that sorts the list to put top ten in order	
			charList.sort(compare_char);
			if(charList.size() > 10)
				charList.pop_back();
		}
	}
	//next 4 if statements check the '\' characters, and adds them to list if applicable
	if(charArray[95] > 0)
	{
		tempPair.first = '\n';
		tempPair.second = charArray[95];
		charList.push_back(tempPair);	
		charList.sort(compare_char);
		if(charList.size() > 10)
			charList.pop_back();
	}
	if(charArray[96] > 0)
	{
		tempPair.first = '\t';
		tempPair.second = charArray[96];
		charList.push_back(tempPair);	
		charList.sort(compare_char);
		if(charList.size() > 10)
			charList.pop_back();
	}
	if(charArray[97] > 0)
	{
		tempPair.first = '\v';
		tempPair.second = charArray[97];
		charList.push_back(tempPair);	
		charList.sort(compare_char);
		if(charList.size() > 10)
			charList.pop_back();
	}
	if(charArray[98] > 0)
	{
		tempPair.first = '\r';
		tempPair.second = charArray[98];
		charList.push_back(tempPair);	
		charList.sort(compare_char);
		if(charList.size() > 10)
			charList.pop_back();
	}
	
	
		
	int count = 0;
	//prints total numer of unique characters, and then lists the top ten characters and number of times used
	cout<<"Total "<<charNum<<" different characters, "<<charList.size()<<" most used characters:"<<endl;
	for (list<pair<char, int> >::iterator it=charList.begin(); it != charList.end(); ++it)
	{	
		cout << "No. " << count << ": ";
		if(it->first == '\n')
			cout << "\\n"; 
		else if(it->first == '\t')
			cout << "\\t"; 
		else if(it->first == '\v')
			cout << "\\v"; 
		else if(it->first == '\r')
			cout << "\\r"; 
		else 
			cout << it->first;	
		cout << "\t\t" << it->second << endl;
		count++;
	}
	
	//traverses map of words, and adds them to list of top ten words if they fit
	for (map<string,pair<int, int> >::iterator itr = wordList.begin(); itr != wordList.end(); ++itr)
	{
		stringList.push_back(*itr);	
		stringList.sort(compare_pair);
		if(stringList.size() > 10)
			stringList.pop_back();
	}	
	count = 0;
	//prints total numer of unique words, and then lists the top ten words and number of times used
	cout<<endl<<"Total "<<wordList.size()<<" different words, "<<stringList.size()<<" most used words:"<<endl;
	for (list<pair<string,pair<int, int> > >::iterator it=stringList.begin(); it != stringList.end(); ++it)
	{	
		cout << "No. " << count << ": " << it->first << "\t\t" << it->second.second << endl;
		count++;
	}
	stringList.clear();
	/*
	//traverses map of numbers, and adds them to list of top ten numbers if they fit
	for (map<string,pair<int, int>>::iterator itr = numList.begin(); itr != numList.end(); ++itr)
	{
		stringList.push_back(*itr);	
		stringList.sort(compare_pair);
		if(stringList.size() > 10)
			stringList.pop_back();
	}	
	count = 0;
	//prints total numer of unique numbers, and then lists the top ten numbers and number of times used
	cout<<endl<<"Total "<<numList.size()<<" different numbers, "<<stringList.size()<<" most used numbers:"<<endl;
	for (list<pair<string,pair<int, int> > >::iterator it=stringList.begin(); it != stringList.end(); ++it)
	{	
		cout << "No. " << count << ": " << it->first << "\t\t" << it->second.second << endl;
		count++;
	}
	*/
	return 0;	
}
//function that takes in the map parameters, if the string is already in the map it increments its counter, if not it adds the string to the map
void insert(string s, int pos, map<string,pair<int, int>> & m)
{
	pair<int, int> tempPair;
	map<string,pair<int,int>>::iterator itr;
	itr = m.find(s);
	tempPair.first = pos;
	tempPair.second = 1;
	
	if(itr == m.end())
		m[s] = tempPair;
	else
		m[s].second++;
}
//function that compares string pairs, first checks the occurance, then checks position as tiebreaker. used in fucntion overload
bool compare_pair(const pair<string, pair<int,int> > & first, const pair<string, pair<int,int> > & second)
{
	if (first.second.second > second.second.second)
		return 1;
	else if(first.second.second == second.second.second && first.second.first < second.second.first)
		return 1;
	else
		return 0;	
}
//function that compares char pairs, uses the occurance to determine compaison. used in fucntion overload
bool compare_char(const pair<char, int> & first, const pair<char, int> & second)
{
	if (first.second > second.second)
		return 1;
	else
		return 0;	
}
