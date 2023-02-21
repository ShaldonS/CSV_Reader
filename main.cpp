#include <cstring>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <queue>
#include <vector>

class CSVReader {
	std::ifstream csv_file;
	std::multimap<std::pair<int, int>, std::string> csv_table;
	std::queue<std::pair<int, int>> formulas;
	int max_commas;
public:
	CSVReader() : max_commas(0) {}
	~CSVReader() {
		if (csv_file.is_open())
			csv_file.close();
	}

	void print_CSV() { // printing SCV table to console
		int row_ = 0;
		int row_old = 0;
		for (auto& it : csv_table) {
			row_ = it.first.first;

			if (row_ != row_old) {
				std::cout << "\b \n";
			}
			if (row_ == 0) {
				std::cout << get_column_name_by_pos(std::stoi(it.second)) << ","; 
			}
			else 
				std::cout << it.second << ",";
			row_old = it.first.first;
		}
		std::cout << "\b \n";
	}

	bool read_CSV_file(const char* filename) { // reading the SCV file (also filling csv_table)
		std::string line("");
		std::string cell_text("");
		std::string col_name = "";

		int row = 1;
		int col = 0;

		csv_file.open(filename);
		if (!csv_file.is_open() || !csv_file.good()) {
			std::cout << "Error file. Exit...\n";
			return false;
		}

		while (std::getline(csv_file, line))
		{
			if (std::find_if(line.begin(), line.end(), 
				[](const char& ch) { return ch != ';'; }) == line.end()) {
				row++;
				continue;
			}

			std::stringstream stream(line);
			int count_of_commas = std::count(line.begin(), line.end(), ';');
			if (count_of_commas > max_commas) {
				max_commas = count_of_commas+1;
			}
			for (int i(0); i < count_of_commas + 1; ++i) {
				std::getline(stream, cell_text, ';');

				if (col == 0) { // adding 0's column
					csv_table.insert(std::make_pair(std::make_pair(row, col), std::to_string(row)));
					++col;
				}
				if (cell_text[0] == '=')
				{
					formulas.push(std::make_pair(row, col));
				}
				else {
					int some_int;
					int succesful_fields = sscanf(cell_text.c_str(), "%d", &some_int);
					if (succesful_fields == 0) {
						std::cout << "Error: Wrong cell on row: " << row << " col: " << col 
							<< ". Not a number or Wrong formula. Exit...\n";
						return false;
					}
					if (cell_text.find('+') != std::string::npos 
						|| cell_text.find('-') != std::string::npos
						|| cell_text.find('/') != std::string::npos 
						|| cell_text.find('*') != std::string::npos) {
						std::cout << "Error: Wrong cell on row: " << row << " col: " << col
							<< ". Wrong formula: missing sign '='. Exit...\n";
						return false;
					}
				}
				csv_table.insert(std::make_pair(std::make_pair(row, col), cell_text));
				col++;
			}
			col = 0;
			row++;
		}
		
		csv_file.close();

		if (!calc_formulas()) {
			return false;
		}
		normilize_CSV_table();
		return true;
	}
private:
	bool calc_formulas() { // calculating the formulas in cells containing it
		int size = formulas.size();
		float new_val = 0.0f;

		std::string expression("");
		std::string operand_1_str(""), operand_2_str("");

		std::stringstream ss;
		ss << std::fixed << std::setprecision(2);

		for (int i(0); i < size; ++i) {
			new_val = 0;
			auto iter = csv_table.find(formulas.front());
			expression = iter->second;

			expression.erase(0, 1); // delete '='

			operand_1_str = "";
			operand_2_str = "";
			for (int j(0); j < expression.size(); ++j) {
				char ch1 = expression[j];
				if (ch1 > 47)
					operand_1_str += ch1;
				else {
					int k = j + 1;
					while (k != expression.size()) {
						operand_2_str += expression[k];
						k++;
					}

					float operand_1(0.0f), operand_2(0.0f);
					std::string row(""), col("");
					for (int l(0); l < operand_1_str.size(); ++l) {
						if (operand_1_str[l] >= 65) {
							col += operand_1_str[l];
						}
						else if (operand_1_str[l] <= 57 && operand_1_str[l] >= 48) {
							row += operand_1_str[l];
						}
					}
					int col_int = get_pos_of_column(col);
					auto it = csv_table.find(std::make_pair(std::stoi(row), col_int));
					if (it != csv_table.end())
						operand_1 = std::atof(it->second.c_str());
					else {
						int some_int;
						int succesful_fields = sscanf(row.c_str(), "%d", &some_int);
						if (succesful_fields == 0) {
							std::cout << "Error: Operand 1 in formula " << expression
								<< " is incorrect. Cell was not found. Exit...\n";
							return false;
						}
						operand_1 = std::atof(row.c_str());
					}
					row = "";
					col = "";

					for (int l(0); l < operand_2_str.size(); ++l) {
						if (operand_2_str[l] >= 65) {
							col += operand_2_str[l];
						}
						else if (operand_2_str[l] <= 57 && operand_2_str[l] >= 48) {
							row += operand_2_str[l];
						}
					}
					col_int = get_pos_of_column(col);
					it = csv_table.find(std::make_pair(std::stoi(row), col_int));
					if (it != csv_table.end())
						operand_2 = std::atof(it->second.c_str());
					else {
						int some_int;
						int succesful_fields = sscanf(row.c_str(), "%d", &some_int);
						if (succesful_fields == 0) {
							std::cout << "Error: Operand 2 in formula " << expression
								<< " is incorrect. Cell was not found. Exit...\n";
							return false;
						}
						operand_2 = std::atof(row.c_str());
					}

					switch (ch1) {
					case 42:
						new_val = operand_1 * operand_2;
						break;
					case 43:
						new_val = operand_1 + operand_2;
						break;
					case 45:
						new_val = operand_1 - operand_2;
						break;
					case 47:
						if (operand_2 == 0) {
							std::cout << "Error: Division by zero. Exit...\n";
							return false;
						}
						new_val = operand_1 / operand_2;
						break;
					}
					continue;
				}
			}

			ss << new_val;
			iter->second = ss.str();
			formulas.pop();
			ss.str(""); // clear stringstream
		}
		return true;
	}

	void normilize_CSV_table() { // adding 0's row (header - A B C D etc.) in csv_table 
		int col = 0;
		for (int i(0); i < max_commas + 1; ++i) {
			csv_table.insert(std::make_pair(std::make_pair(0, col), std::to_string(col)));
			col++;
		}
	}

	int get_pos_of_column(std::string &str) { // getting sequence number of column
		int num = 0;
		for (int i(0); i < str.size(); ++i) {
			num += str[i]-64;
		}
		int a = 0;
		if(str.size() > 1) 
			a = 25 * (str[0] - 64);
		return num+a;
	}

	std::string get_column_name_by_pos(int num) { // get string representation of column by sequence number (position)
		if (num < 1) return "";

		std::string col_name("");
		if (num <= 26) {
			col_name = (num + 64);
		}
		else {
			int count = num / 26;
			int second_letter = num % 26;
			if (second_letter != 0) {
				col_name = (count + 64);
				col_name += (second_letter + 64);
			}
			else {
				col_name = (count - 1 + 64);
				col_name += (26 + 64);
			}
		}

		return col_name;
	}
};

int main(int argc, char* argv[]) {
	const char *filename = argv[1];
	int size = strlen(filename) - 1;
	if (filename[size] != 'v' && filename[size] != 's' && filename[size] != 'c') {
		std::cout << "Error: File format is not CSV. Exit...\n";
	}
	else {
		CSVReader csv_reader;
		if (csv_reader.read_CSV_file(filename)) {
			csv_reader.print_CSV();
		}
	}

	return 0;
}
