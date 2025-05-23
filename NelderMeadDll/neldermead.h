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
#include "json.hpp"

using namespace std;

double evaluateFunction(vector<double> point, char* function);

class element {
public:
	vector<double> point;
	double functionValue;
	element() : point({ 0, 0 }), functionValue(0) {}
	element(vector<double> p, char* function) {
		point = p;
		functionValue = evaluateFunction(p, function);
	}
};

struct NelderMeadParams {
	double reflectionCoeff;
	double contractionCoeff;
	double expansionCoeff;
	double scale;
	double eps;
	int maxSteps;
};

typedef void (*PointsCallback)(double* point);
void sendPoints(PointsCallback callback, vector<element> elements);
vector<element> makeStartSimplex(int varsCount, double scale, vector<double> startingPoint, char* function);
vector<double> operator*(const vector<double>& vec, double scalar);
vector<double> operator/(const vector<double>& vec, double scalar);
vector<double> operator+(const vector<double>& vec1, const vector<double>& vec2);
vector<double> operator-(const vector<double>& vec1, const vector<double>& vec2);
vector<double> calculateMassCenter(vector<element> elements);
bool compare(const element a, const element b);
bool endCheck(double eps, vector<element> elements);
element calculateContraction(vector<element> elements, element reflection, vector<double> massCenter, double contractionCoeff, char* function);
extern "C" MYDLL_API double* findFunctionMinimum(PointsCallback callback, int varsCount, double* startingPointPtr, char* function);
extern "C" MYDLL_API double evaluateFunctionImport(double* pointPtr, int size, char* function);
