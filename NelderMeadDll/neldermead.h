#pragma once

#ifdef MYDLL_EXPORTS
#define MYDLL_API __declspec(dllexport)
#else
#define MYDLL_API __declspec(dllimport)
#endif

#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
#include "tinyexpr.h"

using namespace std;

extern "C" MYDLL_API double evaluateFunction(vector<double> point, string function);

class element {
public:
	vector<double> point;
	double functionValue;
	element() : point({ 0, 0 }), functionValue(0) {}
	element(vector<double> p, string function) {
		point = p;
		functionValue = evaluateFunction(p, function);
	}
};

vector<element> makeStartSimplex(int varsCount, double scale, vector<double> startingPoint, string expression);
vector<double> operator*(const vector<double>& vec, double scalar);
vector<double> operator/(const vector<double>& vec, double scalar);
vector<double> operator+(const vector<double>& vec1, const vector<double>& vec2);
vector<double> operator-(const vector<double>& vec1, const vector<double>& vec2);
vector<double> calculateMassCenter(vector<element> elements);
bool compare(const element a, const element b);
bool endCheck(double eps, vector<element> elements);
element calculateContraction(vector<element> elements, element reflection, vector<double> massCenter, double contractionCoeff, string expression);
extern "C" MYDLL_API vector<double> findFunctionMinimum(int varsCount, vector<double> startingPoint, string function);
