
#include "pch.h"
#include "neldermead.h"

double evaluateFunction(vector<double> point, string function) {
	te_parser tep;
	set<te_variable> variables;
	for (int i = 0; i < point.size(); i++)
		variables.insert({ "x" + to_string(i + 1), &point[i] });
	tep.set_variables_and_functions(variables);
	double result = tep.evaluate(function);
	try {
		if (!tep.success()) throw runtime_error("Incorrect expression");
	}
	catch (const runtime_error& e) {
		cerr << "Error: " << e.what() << endl;
		exit(-1);
	}
	return result;
}

vector<element> makeStartSimplex(int varsCount, double scale, vector<double> startingPoint, string function) {
	vector<element> elements;
	elements.push_back(element(startingPoint, function));
	for (int i = 0; i < varsCount; i++) {
		vector<double> newPoint(varsCount, 0);
		newPoint[i] = scale;
		elements.push_back(element(newPoint, function));
	}
	return elements;
}

vector<double> operator*(const vector<double>& vec, double scalar) {
	vector<double> result(vec.size());
	for (int i = 0; i < vec.size(); i++) {
		result[i] = vec[i] * scalar;
	}
	return result;
}

vector<double> operator/(const vector<double>& vec, double scalar) {
	vector<double> result(vec.size());
	for (int i = 0; i < vec.size(); i++) {
		result[i] = vec[i] / scalar;
	}
	return result;
}

vector<double> operator+(const vector<double>& vec1, const vector<double>& vec2) {
	vector<double> result(vec1.size());
	for (int i = 0; i < vec1.size(); i++) {
		result[i] = vec1[i] + vec2[i];
	}
	return result;
}

vector<double> operator-(const vector<double>& vec1, const vector<double>& vec2) {
	vector<double> result(vec1.size());
	for (int i = 0; i < vec1.size(); i++) {
		result[i] = vec1[i] - vec2[i];
	}
	return result;
}

vector<double> calculateMassCenter(vector<element> elements) {
	vector<double> massCenter(elements.size() - 1);
	for (int i = 0; i < elements.size() - 1; i++)
		massCenter = massCenter + elements[i].point / (double)(elements.size() - 1);
	return massCenter;
}

bool compare(const element a, const element b)
{
	return a.functionValue < b.functionValue;
}

bool endCheck(double eps, vector<element> elements) {
	double sum = 0;
	for (int i = 1; i < elements.size(); i++)
	{
		sum += pow(elements[i].functionValue - elements.front().functionValue, 2);
	}
	return (sqrt(sum / (elements.size() - 1)) <= eps);
}

element calculateContraction(vector<element> elements, element reflection, vector<double> massCenter, double contractionCoeff, string function)
{
	element contraction;
	if (elements.back().functionValue <= reflection.functionValue)
		contraction = element(massCenter + (elements.back().point - massCenter) * contractionCoeff, function);
	else contraction = element(massCenter + (reflection.point - massCenter) * contractionCoeff, function);
	return contraction;
}

string vectorToString(vector<double> point, int number) {
	string str = "X" + to_string(number) + "=(";
	if (number == -1) str = "(";
	for (int i = 0; i < point.size(); ++i) {
		str+= to_string(point[i]);
		if (i < point.size() - 1) str += ", ";
	}
	str += ")";
	return str;
}

vector<double> findFunctionMinimum(int varsCount, vector<double> startingPoint, string function) {
	double reflectionCoeff = 1;
	double contractionCoeff = 0.5;
	double expansionCoeff = 2;
	double scale = 1;
	double eps = 0.001;
	ofstream out;
	out.open("log.txt");
	vector<element> elements = makeStartSimplex(varsCount, scale, startingPoint, function);
	int k = 0;
	for (;;) {
		sort(begin(elements), end(elements), compare);
		if (endCheck(eps, elements) || k == 200) break;
		k++;
		out << "Шаг №" << k << endl;
		out << "Вершины симплекса: " << endl;
		for (int i = 0; i < elements.size(); i++)
		{
			out << vectorToString(elements[i].point,i);
			if (i < elements.size() - 1) out << ", ";
		}
		out << endl;
		vector<double> massCenter = calculateMassCenter(elements);
		element reflection = element(massCenter * (1 + reflectionCoeff) - elements.back().point * reflectionCoeff, function);
		out << "Отражение: " << vectorToString(reflection.point, -1) << endl;
		if (elements.front().functionValue <= reflection.functionValue && reflection.functionValue <= elements.at(elements.size() - 2).functionValue) {
			elements.back() = reflection;
		}
		else if (reflection.functionValue < elements.front().functionValue) {
			element expansion = element(massCenter * (1 - expansionCoeff) + reflection.point * expansionCoeff, function);
			out << "Растяжение: " << vectorToString(expansion.point, -1) << endl;
			if (expansion.functionValue < reflection.functionValue) elements.back() = expansion;
			else elements.back() = reflection;
		}
		else {
			element contraction = calculateContraction(elements, reflection, massCenter, contractionCoeff, function);
			out << "Сжатие: " << vectorToString(contraction.point, -1) << endl;
			if (contraction.functionValue < min(elements.back().functionValue, reflection.functionValue))
				elements.back() = contraction;
			else {
				for (int i = 1; i < elements.size(); i++)
					elements[i] = element(elements[i].point + ((elements.front().point - elements[i].point) / 2.0), function);
			}
		}
	}
	out << "Лучшее решение: " << vectorToString(elements.front().point, -1) << endl;
	out.close();
	return elements.front().point;
}