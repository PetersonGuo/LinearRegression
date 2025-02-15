#include <Python.h>
#include "matplotlib-cpp/matplotlibcpp.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <unordered_map>

namespace plt = matplotlibcpp;

#define x first
#define y second

std::string out_file, testdata_file;		 					// Output File, Test Data File
std::vector<std::pair<double, double>> points;
std::unordered_map<double, std::vector<double>> x_set;
std::vector<double> x_data, y_data, residual_x, residual_y; 	// Scatter Plot

// Read Data from File
void readData()
{
	std::ifstream fs(testdata_file, std::fstream::in);
	try {
		double x,y;
		while (fs >> x >> y) {
			double x,y;
			x_data.push_back(x);
			y_data.push_back(y);
			points.push_back({x,y});
		}
	}
	catch (std::invalid_argument) {throw;}
	catch (std::ios_base::failure) {throw;}
	catch (...) {throw;}

	fs.close();
}

void calculate_regression(double &m, double &b, double &r)
{
	unsigned long long N = points.size();
	double sx = 0, sy = 0, sxy = 0, sxx = 0;

	for (auto it = points.begin(); it != points.end(); ++it) {
		sx += it->first;
		sy += it->second;
		sxx += it->first*it->first;
		sxy += it->first*it->second;
	}

	m = (N*sxy - sx*sy)/(N*sxx - sx*sx);
	b = (sy - m*sx)/N;
	r = sxy/(sx*sy);
}

void calculate_residual(const double &m, const double &b)
{
	for (auto it : points)
		x_set[it.first].push_back(it.second);

	for (auto it = x_set.begin(); it != x_set.end(); ++it) {
		double yhat = 0;
		for (auto i : it->second) yhat += i;
		yhat /= it->second.size();
		residual_x.push_back(it->first);
		residual_y.push_back(yhat - (m * it->first + b));
	}
}

void initializePython() {
    // Initialize Python interpreter
    Py_Initialize();

    // Add the virtual environment to sys.path
    PyRun_SimpleString("import sys; sys.path.insert(0, '/app/venv/lib/python3.13/site-packages')");

    // Initialize NumPy C-API
    if (_import_array() < 0) {
        PyErr_Print();
        throw std::runtime_error("Failed to initialize NumPy C-API!");
    }

    std::cout << "Python and NumPy initialized successfully!" << std::endl;
}

void finalizePython() {
    // Finalize Python interpreter
    Py_Finalize();
}

// Main Function
int main(int argc, char *argv[])
{
	if (argc < 3) goto help;

	// Check for help flag
	for (int i = 1; i < argc; i++)
		if (argv[i] == "-h" || argv[i] == "--help")
			goto help;
	goto program;

help:
	std::cout << "Usage: ./main [testdata_file] [output_file]" << std::endl;
	return 0;

program:
	testdata_file = argv[1]; 	// Default Input File
	out_file = argv[2];			// Default Output File

	try {
			initializePython();

			// Your main code here
			std::cout << "NumPy and Matplotlib imported successfully!" << std::endl;

			finalizePython();
	} catch (const std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
			finalizePython();
			return 1;
	}

	readData();
	plt::scatter(x_data, y_data, 10.0, {{"c", "r"}});

	// Calculate Linear Regression
	double m, b, r;
	calculate_regression(m, b, r);

	// Print linear equation to file
	std::ofstream fs(out_file, std::ios::app);
	fs << "y = " << (m == 1 ? "" : std::to_string(m)) << (b > 0 ? "x + " + std::to_string(abs(b)) : b == 0 ? "x" : ("x - " + std::to_string(abs(b)))) << "\nStrength: " << r << '\n';
	std::cout << "Accuracy: " << r << '\n';

	// Create Linear Regression Line
	std::sort(x_data.begin(), x_data.end());
	std::sort(y_data.begin(), y_data.end());

	std::vector<double> xs, ys;
	for (long long i = -abs(x_data[0]) * 8; i <= abs(x_data[x_data.size()-1]) * 8; ++i) {
		xs.push_back(i);
		ys.push_back(m*i+b);
	}

	// Create Scatter Plot
	plt::named_plot("y = " + (m == 1 ? "" : std::to_string(m)) + (b > 0 ? "x + " + std::to_string(abs(b)) : b == 0 ? "x" : ("x - " + std::to_string(abs(b)))), xs, ys);
	plt::xlim(x_data[0] - x_data[0] * 1.5, x_data[x_data.size()-1] + x_data[x_data.size()-1] * 1.5);
	plt::ylim(y_data[0] - y_data[0] * 1.5, y_data[y_data.size()-1] + y_data[y_data.size()-1] * 1.5);
	plt::grid(1);
	plt::legend();
	plt::save("plot.png");
	plt::close();

	calculate_residual(m, b);

	// Create Residual Plot
	plt::named_plot("Residual", residual_x, residual_y);
	plt::scatter(residual_x, residual_y, 10.0, {{"c", "g"}});
	plt::save("residual.png");
	plt::close();

	std::cout << "Interpolate/Extrapolate:\n\n Enter x points:" << std::endl;
	double x;
	while (std::cin >> x) {
		double yhat = 0;
		for (auto &i : x_set[x]) yhat += i;
		yhat /= x_set[x].size();
		std::cout << "y est: " << m * x + b << "\nResidual: " << yhat - (m * x + b) << '\n';
	}

	fs.close();
	return 0;
}
