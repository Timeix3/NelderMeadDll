#include "pch.h"
#include "neldermead.h"

double evaluateFunctionImport(double* pointPtr, int size, char* function) {
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

double evaluateFunction(vector<double> point, char* function) {
	te_parser tep;
	set<te_variable> variables;
	for (int i = 0; i < point.size(); i++)
		variables.insert({ "x" + to_string(i + 1), &point[i] });
	tep.set_variables_and_functions(variables);
	double result = tep.evaluate(function);
    if (!tep.success()) throw runtime_error("Incorrect expression");
	return result;
}

vector<element> makeStartSimplex(int varsCount, double scale, vector<double> startingPoint, char* function) {
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

element calculateContraction(vector<element> elements, element reflection, vector<double> massCenter, double contractionCoeff, char* function)
{
	element contraction;
	if (elements.back().functionValue <= reflection.functionValue)
		contraction = element(massCenter + (elements.back().point - massCenter) * contractionCoeff, function);
	else contraction = element(massCenter + (reflection.point - massCenter) * contractionCoeff, function);
	return contraction;
}

string printVector(vector<double> point, int number) {
	string str = "X" + to_string(number) + "=(";
	if (number == -1) str = "(";
	for (int i = 0; i < point.size(); ++i) {
		str+= to_string(point[i]);
		if (i < point.size() - 1) str += ", ";
	}
	str += ")";
	return str;
}

void sendPoints(PointsCallback callback, vector<element> elements) {
	for (int i = 0; i < elements.size(); i++) {
		callback(elements[i].point.data());
	}
}

void to_json(nlohmann::json& j, const NelderMeadParams& p) {
	j = nlohmann::json{
		{"reflectionCoeff", p.reflectionCoeff},
		{"contractionCoeff", p.contractionCoeff},
		{"expansionCoeff", p.expansionCoeff},
		{"scale", p.scale},
		{"eps", p.eps},
		{"maxSteps", p.maxSteps}
	};
}

void from_json(const nlohmann::json& j, NelderMeadParams& p) {
	j.at("reflectionCoeff").get_to(p.reflectionCoeff);
	j.at("contractionCoeff").get_to(p.contractionCoeff);
	j.at("expansionCoeff").get_to(p.expansionCoeff);
	j.at("scale").get_to(p.scale);
	j.at("eps").get_to(p.eps);
	j.at("maxSteps").get_to(p.maxSteps);
}


NelderMeadParams loadConfig() {
	NelderMeadParams params;
	try {
		ifstream in("config.json");
		nlohmann::json j;
		in >> j;
		params = j.get<NelderMeadParams>();
		in.close();
	}
	catch (...) {
		params = { 1.0, 0.5, 2.0, 1.0, 0.001, 500 };
		nlohmann::json j = params;
		ofstream out("config.json");
		out << j.dump(4);
		out.close();
	}
	return params;
}

double* findFunctionMinimum(PointsCallback callback, int varsCount, double* startingPointPtr, char* function) {
	NelderMeadParams params = loadConfig();
	vector<double> startingPoint(startingPointPtr, startingPointPtr + varsCount);
	ofstream out;
	out.open("log.txt");
	vector<element> elements = makeStartSimplex(varsCount, params.scale, startingPoint, function);
	int k = 0;
	for (;;) {
		sort(begin(elements), end(elements), compare);
		if (callback != nullptr)
			sendPoints(callback, elements);
		if (endCheck(params.eps, elements) || k == params.maxSteps) break;
		k++;
		out << "Шаг №" << k << endl;
		out << "Вершины симплекса: " << endl;
		for (int i = 0; i < elements.size(); i++)
		{
			out << printVector(elements[i].point,i);
			if (i < elements.size() - 1) out << ", ";
		}
		out << endl;
		vector<double> massCenter = calculateMassCenter(elements);
		element reflection = element(massCenter * (1 + params.reflectionCoeff) - elements.back().point * params.reflectionCoeff, function);
		out << "Отражение: " << printVector(reflection.point, -1) << endl;
		if (elements.front().functionValue <= reflection.functionValue && reflection.functionValue <= elements.at(elements.size() - 2).functionValue) {
			elements.back() = reflection;
		}
		else if (reflection.functionValue < elements.front().functionValue) {
			element expansion = element(massCenter * (1 - params.expansionCoeff) + reflection.point * params.expansionCoeff, function);
			out << "Растяжение: " << printVector(expansion.point, -1) << endl;
			if (expansion.functionValue < reflection.functionValue) elements.back() = expansion;
			else elements.back() = reflection;
		}
		else {
			element contraction = calculateContraction(elements, reflection, massCenter, params.contractionCoeff, function);
			out << "Сжатие: " << printVector(contraction.point, -1) << endl;
			if (contraction.functionValue < min(elements.back().functionValue, reflection.functionValue))
				elements.back() = contraction;
			else {
				for (int i = 1; i < elements.size(); i++)
					elements[i] = element(elements[i].point + ((elements.front().point - elements[i].point) / 2.0), function);
			}
		}
	}
	out << "Лучшее решение: " << printVector(elements.front().point, -1) << endl;
	out.close();
	double* res = new double[varsCount];
	std::copy(elements.front().point.begin(), elements.front().point.end(), res);
	return res;
}