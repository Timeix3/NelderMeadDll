#pragma once

#include <iostream>
#include <fstream>
#include "json.hpp"

void to_json(nlohmann::json& j, const nelderMeadParams& p) {
	j = nlohmann::json{
		{"reflectionCoeff", p.reflectionCoeff},
		{"contractionCoeff", p.contractionCoeff},
		{"expansionCoeff", p.expansionCoeff},
		{"scale", p.scale},
		{"eps", p.eps},
		{"maxSteps", p.maxSteps}
	};
}

void from_json(const nlohmann::json& j, nelderMeadParams& p) {
	j.at("reflectionCoeff").get_to(p.reflectionCoeff);
	j.at("contractionCoeff").get_to(p.contractionCoeff);
	j.at("expansionCoeff").get_to(p.expansionCoeff);
	j.at("scale").get_to(p.scale);
	j.at("eps").get_to(p.eps);
	j.at("maxSteps").get_to(p.maxSteps);
}

nelderMeadParams loadConfig(string filename = "config.json") {
	nelderMeadParams params;
	try {
		ifstream in(filename);
		nlohmann::json j;
		in >> j;
		params = j.get<nelderMeadParams>();
		in.close();
	}
	catch (...) {
		params = { 1.0, 0.5, 2.0, 1.0, 0.001, 500 };
		nlohmann::json j = params;
		ofstream out(filename);
		out << j.dump(4);
		out.close();
	}
	return params;
}