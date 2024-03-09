#pragma once
#include <string>
#include <iostream>

using namespace std;

class Cat {
public:
	Cat():name("Default Name") 
	{
		cout << "Constructor of Cat: " << name << endl;
	}
	Cat(string i_name) : name(i_name) 
	{
		cout << "Constructor of Cat: " << name << endl;
	}
	~Cat() {
		cout << "Destructor of Cat: " << name << endl;
	}

	void SetCatName(string i_name) { name = i_name; }
	string GetCatName() { return name; }
	void CatInfo() 
	{
		cout << "Cat Name: " << name << endl;
	}

private:
	string name;
};