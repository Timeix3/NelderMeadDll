#pragma once

#ifdef MYDLL_EXPORTS
#define MYDLL_API __declspec(dllexport)
#else
#define MYDLL_API __declspec(dllimport)
#endif

#include <vector>
#include <fstream>

using namespace std;

typedef void (*pointsCallback)(double* point);

struct nelderMeadParams {
	double reflectionCoeff;
	double contractionCoeff;
	double expansionCoeff;
	double scale;
	double eps;
	int maxSteps;
};

extern "C" MYDLL_API double evaluateFunction(double* pointPtr, int size, char* function);
extern "C" MYDLL_API double* findFunctionMinimum(pointsCallback callback, int varsCount, double* startingPointPtr, char* function);

class element {
public:
	vector<double> point;
	double functionValue;
	element() : point({ 0, 0 }), functionValue(0) {}
	element(vector<double> p, char* function) {
		point = p;
		functionValue = evaluateFunction(p.data(), p.size(), function);
	}
};

class nelderMead {
public:
	nelderMeadParams params;
	vector<element> simplex;
	ofstream out;
	pointsCallback callback;
	char* function;
	nelderMead(pointsCallback callback, char* function);
	vector<double> start(int varsCount, double* startingPointPtr);
	void sendPoints();
	string printVector(vector<double> point, int number);
	void makeStartSimplex(int varsCount, vector<double> startingPoint);
	vector<double> calculateMassCenter();
	void changeSimplex();
	void globalContraction();
	bool endCheck(double eps, vector<element> simplex);
	element calculateContraction(element reflection, vector<double> massCenter);
	void logSimplex(int k);
};

