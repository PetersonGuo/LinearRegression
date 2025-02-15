#include "matplotlib-cpp/matplotlibcpp.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>

namespace plt = matplotlibcpp;

#define x first
#define y second

typedef long double ld;

std::string out_file, testdata_file;		 					// Output File, Test Data File
std::vector<std::pair<ld,ld>> points;
std::vector<ld> scatter_x, scatter_y, residual_x, residual_y; 	// Scatter Plot
std::vector<ld> xs, ys;											// Linear Regression Line

// Read Data from File
void readData(ld &avgx, ld &avgy) {
	std::ifstream fs(testdata_file, std::fstream::in);

	// Calculate Average
	ld t;

	fs >> t;
	
	// Read Data
	try {
		do {
			ls x,y;
			fs >> x;
			avgx += x;
			scatter_x.push_back(x);

			fs >> y;
			avgy += y;
			scatter_y.push_back(y);

			points.push_back({x,y});
		} while (!fs.eof());
	}
	catch (std::invalid_argument) {throw;}
	catch (std::ios_base::failure) {throw;}
	catch (...) {throw;}

	// Calculate Average
	avgx /= scatter_x.size();
	avgy /= scatter_y.size();

	fs.close();
}

// Main Function
int main(int argc, char *argv[]) {
	if (argc < 3) goto help;
	// Check for help flag
	for (int i = 1; i < argc; i++) {
		if (argv[i] == "-h" || argv[i] == "--help") {
			goto help;
		}
	}
	
	testdata_file = argv[1]; 	// Default Input File
	out_file = argv[2];			// Default Output File

	// Read Data
	ld avgx = 0, avgy = 0;
	readData(avgx, avgy);
	plt::scatter(scatter_x, scatter_y, 10.0, {{"c", "r"}});

	// Calculate Linear Regression
	ld sx = 0, sy = 0, sxy = 0;
	for (auto it = points.begin(); it != points.end(); ++it) {
		ld currX = it->x - avgx, currY = it->y - avgy;
		sx += currX * currX;
		sy += currY * currY;
		sxy += currX * currY;
	}

	// Calculate Linear Regression
	ld m = sxy / sx, b = avgy - m * avgx;
	sxy = sxy / (scatter_x.size()-1);
	sx = sqrt(sx / (scatter_x.size()-1));
	sy = sqrt(sy / (scatter_y.size()-1));
	ld r = sxy / (sx * sy);

	// Print linear equation to file
	std::ofstream fs(out_file, std::ios::app);
	fs << "y = " << (m == 1 ? "" : std::to_string(m)) << (b > 0 ? "x + " + std::to_string(abs(b)) : b == 0 ? "x" : ("x - " + std::to_string(abs(b)))) << "\nStrength: " << r << '\n';
	std::cout << "Accuracy: " << r << '\n';

	// Create Linear Regression Line
	std::sort(scatter_x.begin(), scatter_x.end());
	std::sort(scatter_y.begin(), scatter_y.end());
	for (long long i = -abs(scatter_x[0]) * 8; i <= abs(scatter_x[scatter_x.size()-1]) * 8; i++) {
		xs.push_back(i);
		ys.push_back(m*i+b);
	}

	// Create Scatter Plot
	plt::named_plot("y = " + (m == 1 ? "" : std::to_string(m)) + (b > 0 ? "x + " + std::to_string(abs(b)) : b == 0 ? "x" : ("x - " + std::to_string(abs(b)))), xs, ys);
	plt::xlim(scatter_x[0] - scatter_x[0] * 1.5, scatter_x[scatter_x.size()-1] + scatter_x[scatter_x.size()-1] * 1.5);
	plt::ylim(scatter_y[0] - scatter_y[0] * 1.5, scatter_y[scatter_y.size()-1] + scatter_y[scatter_y.size()-1] * 1.5);
	plt::grid(1);
	plt::legend();
	plt::show();

	// Create Residual Plot
	/*for (auto it = points.begin(); it != points.end(); it++) {
		ld yhat = 0;
		for (auto &i : map[it->x]) yhat += i;
		yhat /= map[it->x].size();
		residual_x.push_back(it->x);
		residual_y.push_back(yhat - (m * it->x + b));
	}*/

	// Create Residual Plot
	plt::named_plot("Residual", residual_x, residual_y);
	plt::scatter(residual_x, residual_y, 10.0, {{"c", "g"}});
	plt::show();

	ld x;
	while (std::cin >> x) {
		ld yhat = 0;
		for (auto &i : map[x]) yhat += i;
		yhat /= map[x].size();
		std::cout << "y est: " << m * x + b << "\nResidual: " << yhat - (m * x + b) << '\n';
	}

	fs.close();
	return 0;

help:
	std::cout << "Usage: ./run.sh [file_name] -o [output_file] -t [testdata_file]\n";
	return 0;
}
