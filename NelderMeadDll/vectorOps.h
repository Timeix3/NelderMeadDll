#pragma once

#include <vector>

using namespace std;

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