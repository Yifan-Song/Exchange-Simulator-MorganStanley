// ConsoleApplication27.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
using namespace std;
bool same = false;

void search(int n)
{
	ifstream d;
	d.open("C:\\Users\\Bo\\Desktop\\C++\\lab3\\serafini\\WordLadder\\res\\EnglishWords.txt");
	string s1;
	vector<string> v1;
	vector<int> v2;
	while (!d.eof())
	{
		same = false;
		getline(d, s1);
		if (s1.length() < n)continue;
		string suffix = s1.substr(s1.length() - n, n);
		for (int i = 0;i < v1.size();i++)
		{
			if (suffix == v1[i])
			{
				v2[i]++;
				same = true;
			}
		}
		if (!same)
		{
			v1.push_back(suffix);
			v2.push_back(1);
		}
	}
	vector<int> tempv = v2;
	sort(tempv.begin(), tempv.end());
	int limit = 11;
	if (v2.size() < 10)limit = v2.size() + 1;
	for (int i = 1;i < limit;i++)
	{
		int n = tempv[tempv.size() - i];
		vector<int>::iterator iter = std::find(v2.begin(), v2.end(), n);
		v2[distance(v2.begin(), iter)] = -1;
		cout << v1[distance(v2.begin(), iter)] << "\t" <<n<<"\n";
	}
}

void Search(string Suffix)
{
	int counter = 0;
	ifstream d;
	d.open("C:\\Users\\Bo\\Desktop\\C++\\lab3\\serafini\\WordLadder\\res\\EnglishWords.txt");
	string s1;
	vector<string> v;
	while (!d.eof())
	{
		getline(d, s1);
		if (s1.length() < Suffix.length())continue;
		string suffix = s1.substr(s1.length() - Suffix.length(), Suffix.length());
		if (suffix == Suffix)
		{
			counter++;
			v.push_back(s1);
		}
	}
	sort(v.begin(), v.end());
	reverse(v.begin(), v.end());
	cout << counter << endl;
	for (auto i : v)cout << i<<"\n";
}

int main()
{
	int input;
	cout << "Please enter the number:";
	cin >> input;
	search(input);
	string in;
	cout << "Please enter the suffix:";
	cin >> in;
	Search(in);
    return 0;
}