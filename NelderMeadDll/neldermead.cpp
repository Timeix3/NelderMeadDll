#include <iostream>
#include <fstream>
#include "pch.h"
#include "neldermead.h"
#include "vectorOps.h"
#include "jsonSerializer.h"
#include "tinyexpr.h"

double* findFunctionMinimum(pointsCallback callback, int varsCount, double* startingPointPtr, char* function) {
	nelderMead* nelderMeadMethod = new nelderMead(callback, function);
	vector<double> resultPoint = nelderMeadMethod->start(varsCount, startingPointPtr);
	double* res = new double[varsCount];
	std::copy(resultPoint.begin(), resultPoint.end(), res);
	return res;
}

double evaluateFunction(double* pointPtr, int size, char* function) {
	te_parser tep;
	set<te_variable> variables;
	vector<double> point(pointPtr, pointPtr + size);
	for (int i = 0; i < point.size(); i++)
		variables.insert({ "x" + to_string(i + 1), &point[i] });
	tep.set_variables_and_functions(variables);
	double result = tep.evaluate(function);
    if (!tep.success()) throw runtime_error("Incorrect expression");
	return result;
}

writer* nelderMead::chooseOutput() {
	if (params.outputType == "txt") {
		return new txtWriter("log");
	}
	else if (params.outputType == "html") {
		return new htmlWriter("log");
	}
	else throw runtime_error("Incorrect output type");
}

nelderMead::nelderMead(pointsCallback callback, char* function):
	params(loadConfig()),
	output(chooseOutput()),
	callback(callback),
	function(function) {}

vector<double> nelderMead::start(int varsCount, double* startingPointPtr)
{
	vector<double> startingPoint(startingPointPtr, startingPointPtr + varsCount);
	makeStartSimplex(varsCount, startingPoint);
	for (int k = 0; k < params.maxSteps; k++) {
		std::sort(simplex.begin(), simplex.end(),
			[](const element& a, const element& b) {
				return a.functionValue < b.functionValue;
			}
		);
		if (callback != nullptr) sendPoints();
		if (endCheck(params.eps, simplex)) break;
		logSimplex(k);
		changeSimplex();
	}
	output->write("Лучшее решение: " + printVector(simplex.front().point, -1));
	output->closeFile();
	return simplex.front().point;
}

void nelderMead::changeSimplex()
{
	vector<double> massCenter = calculateMassCenter();
	element reflection = element(massCenter * (1 + params.reflectionCoeff) - simplex.back().point * params.reflectionCoeff, function);
	output->write("Отражение: " + printVector(reflection.point, -1));
	if (isReflectionAcceptable(reflection)) {
		simplex.back() = reflection;
	}
	else if (isExpansionNeeded(reflection)) {
		performExpansion(massCenter, reflection);
	}
	else {
		performContraction(reflection, massCenter);
	}
}

void nelderMead::performContraction(element& reflection, std::vector<double>& massCenter)
{
	element contraction = calculateContraction(reflection, massCenter);
	output->write("Сжатие: " + printVector(contraction.point, -1));
	if (contraction.functionValue < min(simplex.back().functionValue, reflection.functionValue))
		simplex.back() = contraction;
	else globalContraction();
}

void nelderMead::performExpansion(std::vector<double>& massCenter, element& reflection)
{
	element expansion = element(massCenter * (1 - params.expansionCoeff) + reflection.point * params.expansionCoeff, function);
	output->write("Растяжение: " + printVector(expansion.point, -1));
	if (expansion.functionValue < reflection.functionValue) simplex.back() = expansion;
	else simplex.back() = reflection;
}

bool nelderMead::isExpansionNeeded(element& reflection)
{
	return reflection.functionValue < simplex.front().functionValue;
}

bool nelderMead::isReflectionAcceptable(element& reflection)
{
	return simplex.front().functionValue <= reflection.functionValue && reflection.functionValue <= simplex.at(simplex.size() - 2).functionValue;
}

void nelderMead::globalContraction()
{
	for (int i = 1; i < simplex.size(); i++)
		simplex[i] = element(simplex[i].point + ((simplex.front().point - simplex[i].point) / 2.0), function);
}

bool nelderMead::endCheck(double eps, vector<element> simplex)
{
	double sum = 0;
	for (int i = 1; i < simplex.size(); i++)
	{
		sum += pow(simplex[i].functionValue - simplex.front().functionValue, 2);
	}
	double dist = sqrt(sum / (simplex.size() - 1));
	return (dist <= eps);
}

element nelderMead::calculateContraction(element reflection, vector<double> massCenter)
{
	element contraction;
	if (simplex.back().functionValue <= reflection.functionValue)
		contraction = element(massCenter + (simplex.back().point - massCenter) * params.contractionCoeff, function);
	else contraction = element(massCenter + (reflection.point - massCenter) * params.contractionCoeff, function);
	return contraction;
}

void nelderMead::sendPoints()
{
	for (int i = 0; i < simplex.size(); i++) {
		callback(simplex[i].point.data());
	}
}

string nelderMead::printVector(vector<double> point, int number)
{
	string str = "X" + to_string(number) + "=(";
	if (number == -1) str = "(";
	for (int i = 0; i < point.size(); ++i) {
		str += to_string(point[i]);
		if (i < point.size() - 1) str += ", ";
	}
	str += ")";
	return str;
}

void nelderMead::makeStartSimplex(int varsCount, vector<double> startingPoint)
{
	simplex.push_back(element(startingPoint, function));
	for (int i = 0; i < varsCount; i++) {
		vector<double> newPoint(startingPoint);
		newPoint[i] += params.scale;
		simplex.push_back(element(newPoint, function));
	}
}

vector<double> nelderMead::calculateMassCenter()
{
	vector<double> massCenter(simplex.size() - 1);
	for (int i = 0; i < simplex.size() - 1; i++)
		massCenter = massCenter + simplex[i].point / (double)(simplex.size() - 1);
	return massCenter;
}

void nelderMead::logSimplex(int k)
{
	output->write("Шаг №" + to_string(k));
	output->write("Вершины симплекса: ");
	string data;
	for (int i = 0; i < simplex.size(); i++)
	{
		data += printVector(simplex[i].point, i);
		if (i < simplex.size() - 1) data += ", ";
	}
	output->write(data);
}

