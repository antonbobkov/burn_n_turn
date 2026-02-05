#include <iostream>
#include <fstream>
#include <string>

using namespace std;

string sFunctorBaseName = "ThreadFunctor";
string sFunctorName = "ThreadFunctor";
string sTemplatedClasses = "T";
string sFnArgName = "fn";
string sArgName = "arg";
string sFnReturnType = "void";
string sTab = "\t";

void GenerateFunctorBase(ostream& ofs)
{
	ofs << "struct " << sFunctorBaseName << "\n{\n";
	ofs << sTab << "virtual void Apply()=0;\n\n";
	ofs << sTab << "virtual ~" << sFunctorBaseName << "(){};\n};\n\n";
}

void GenerateSystemThread(ostream& ofs)
{
	ofs << "void SystemNewThread(" << sFunctorBaseName << "* pF);\n\n";
}

void GenerateList(ostream& ofs, string sPrefix, int n)
{
	for(int i = 0; i < n; ++i)
	{
		ofs << sPrefix << i;
		if(i != n - 1)
			ofs << ", ";
	}
}

void GenerateListFancy(ostream& ofs, string sPrefix, string sMidFix, string sPostFix, bool bSep, int n)
{
	for(int i = 0; i < n; ++i)
	{
		ofs << sPrefix << i << sMidFix << i << sPostFix;
		if(i != n - 1 && bSep)
			ofs << ", ";
	}
}

void GenerateFunctorClass(ostream& ofs, int n)
{
	if(n != 0)
	{
		ofs << "template <";

		GenerateList(ofs, "class " + sTemplatedClasses, n);

		ofs << ">\n";
	}

	ofs << "struct " << sFunctorName << n << ": public " << sFunctorBaseName << "\n{\n";

	ofs << sTab << "void (*" << sFnArgName << ")(";

	GenerateList(ofs, sTemplatedClasses, n);

	ofs << ");\n\n";

	GenerateListFancy(ofs, sTab + sTemplatedClasses, " " + sArgName, ";\n", false, n);

	ofs << "\n";

	ofs << sTab << sFunctorName << n << "(";
	
	ofs << "void (*" << sFnArgName + "_" << ")(";

	GenerateList(ofs, sTemplatedClasses, n);

	ofs << ")";
	if(n != 0)
		ofs << ", ";

	GenerateListFancy(ofs, sTemplatedClasses, " " + sArgName, "_", true, n);

	ofs << ")\n";

	ofs << sTab << sTab << ":" << sFnArgName << "(" << sFnArgName << "_)";
	if(n != 0)
		ofs << ", ";

	GenerateListFancy(ofs, sArgName, "(" + sArgName, "_)", true, n);

	ofs << "{}\n\n";

	ofs << sTab << "/*virtual*/ void Apply()\n";
	ofs << sTab << "{\n";
	ofs << sTab << sTab << sFnArgName << "(";
	GenerateList(ofs, sArgName, n);
	ofs << ");\n";
	ofs << sTab << "}\n";
	ofs << "};\n\n";
};

void GenerateFunction(ostream& ofs, int n)
{
	if(n != 0)
	{
		ofs << "template <";

		GenerateList(ofs, "class " + sTemplatedClasses, n);

		ofs << ">\n";
	}

	ofs << "void NewThread(";

	ofs << "void (*" << sFnArgName << ")(";

	GenerateList(ofs, sTemplatedClasses, n);

	ofs << ")";
	if(n != 0)
		ofs << ", ";

	GenerateListFancy(ofs, sTemplatedClasses, " " + sArgName, "", true, n);

	ofs << ")\n{\n";

	ofs << sTab << sFunctorBaseName << "* pF = new " << sFunctorName << n;
	
	if(n != 0)
	{
		ofs << "<";

		GenerateList(ofs, sTemplatedClasses, n);

		ofs << ">";
	}
	
	ofs << "(" << sFnArgName;
	
	if(n != 0)
		ofs << ", ";

	GenerateList(ofs, sArgName, n);

	ofs << ");\n";
	ofs << sTab << "SystemNewThread(pF);\n}\n\n";

};

int main()
{
	int n = 10;
	
	ofstream ofs("../../GenTui.h");

	GenerateFunctorBase(ofs);
	GenerateSystemThread(ofs);
	
	for(int i = 0; i <= n; ++i)
	{
		GenerateFunctorClass(ofs, i);
		GenerateFunction(ofs, i);
	}

	return 0;
}