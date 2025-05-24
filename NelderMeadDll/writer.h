#pragma once

#include <iostream>
#include<fstream>

using namespace std;

class writer {
public:
	virtual void write(string data) = 0;
	virtual void closeFile() = 0;
	virtual ~writer() = default;
};

class txtWriter: public writer {
private:
	ofstream file;
	void openFile(string filename) {
		if (file.is_open()) {
			closeFile();
		}
		file.open(filename);
	}
public:
	txtWriter(string filename) {
		openFile(filename + ".txt");
	}
	~txtWriter() override {
		closeFile();
	}
	void write(string data) override {
		file << data << endl;
	}
	void closeFile() override {
		if (file.is_open()) {
			file.close();
		}
	}
};

class htmlWriter : public writer {
private:
	ofstream file;
	void openFile(string filename) {
		if (file.is_open()) {
			closeFile();
		}
		file.open(filename);
		file << "<html>" << endl << "<head>" << endl;
		file << "<title>Log</title>" << endl << "</head>" << endl;
		file << "<body>" << endl;
	}
public:
	htmlWriter(string filename) {
		openFile(filename+".html");
	}
	~htmlWriter() override {
		closeFile();
	}
	void write(string data) override {
		if (file.is_open())
			file << "<p>" + data + "</p>" << endl;
	}
	void closeFile() override {
		if (file.is_open()) {
			file << "</body>" << endl << "</html>";
			file.close();
		}
	}
};