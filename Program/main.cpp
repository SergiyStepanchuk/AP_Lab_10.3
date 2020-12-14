// Lab_10_3
#include <iostream>
#include <iomanip>
#include <fstream>
#include <windows.h>
#include <string>
#include <algorithm>
#include <fstream>
using namespace std;

// Console 

namespace cons {

	COORD GetBufferSize() {
		static HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbInfo;
		GetConsoleScreenBufferInfo(handle, &csbInfo);
		return { csbInfo.srWindow.Right - csbInfo.srWindow.Left ,
				csbInfo.srWindow.Bottom - csbInfo.srWindow.Top };
	}

	const COORD size = GetBufferSize();

	void clear() {
		system("cls");
	}

	void gotoxy(const COORD pos) {
		static HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleCursorPosition(handle, pos);
	}

	COORD getxy() {
		static HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!GetConsoleScreenBufferInfo(handle, &csbi))
			return { -1, -1 };
		return csbi.dwCursorPosition;
	}

	void clearto(const COORD pos) {
		COORD current_pos = getxy();
		while (current_pos.Y >= pos.Y)
		{
			if (current_pos.Y > pos.Y) {
				gotoxy({ 0, current_pos.Y });
				for (int i = 0; i < size.X; i++)
					cout << ' ';
			}
			else if (current_pos.Y == pos.Y) {
				gotoxy({ pos.X, current_pos.Y });
				for (int i = 0; i <= size.X - pos.X; i++)
					cout << ' ';
			}

			current_pos.Y--;
		}
		gotoxy(pos);
	}

	void change_cusor_visibility(const bool& rst) {
		static HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_CURSOR_INFO structCursorInfo;
		GetConsoleCursorInfo(handle, &structCursorInfo);
		structCursorInfo.bVisible = rst;
		SetConsoleCursorInfo(handle, &structCursorInfo);
	}

	template <typename T>
	T input(bool (*check)(T& temp, char* err), const bool& rom, const char* text, ...) {
		COORD start[2] = { getxy() };
		char out[256] = { 0 }, err[256] = { 0 };
		T temp;

		va_list  args;
		va_start(args, text);
		vsprintf_s(out, 255, text, args);
		va_end(args);
		cout << out;
		start[1] = getxy();
		if (check == nullptr)
			check = [](T& temp, char* err) -> bool { return !cin.fail(); };
		bool last = true;
		do {
			if (!last) {
				cin.clear();
				cin.ignore();
				last = true;
			}
			if (err[0] != '\0') {
				change_cusor_visibility(false);
				clearto(start[0]);
				cout << err << endl;
				err[0] = '\0';
				cout << out;
				start[1] = getxy();
				change_cusor_visibility(true);
			}
			else clearto(start[1]);
			cin >> temp;
		} while (last = check(temp, err), !last);
		if (rom)
			clearto(start[0]);
		return temp;
	}
}

struct Bill {
	string r_r_payer;
	string r_r_recipient;
	float sum;
};

Bill* InitBill() {
	cout << "Adding new bill: " << endl;
	Bill* bill = new Bill();
	bill->r_r_payer = cons::input<string>(nullptr, false, "Input payer IBAN: ");
	bill->r_r_recipient = cons::input<string>(nullptr, false, "Input recipient IBAN: ");
	bill->sum = cons::input<float>([](float& temp, char* err)-> bool {
		if (cin.fail() ||
			temp <= 0)
		{
			sprintf_s(err, 255, "Incorrect money (money > 0) (%f)", temp);
			return false;
		}
		return true;
		}, false, "Input transfered money: ");
	return bill;
}

float get_bill_by_payer(fstream &file, const string& payer) {
	string tmp, r;
	file.clear();
	file.seekg(0, ios_base::beg);
	while (!file.eof()) {
		getline(file, r);
		if (r.length() == 0)
			continue;
		getline(file, tmp);
		getline(file, tmp);
		if (r == payer)
			return atof(tmp.c_str());
	}
	return -1;
}

void draw_bills(fstream& file) {
	cout << "============================================================================================" << endl;
	cout << "| Id | Payer IBAN                         | Recipient IBAN                     | Sum       |" << endl;
	cout << "--------------------------------------------------------------------------------------------" << endl;
	file.clear();
	file.seekg(0);
	Bill tmp;
	int id = 1;
	while (!file.eof()) {
		
		getline(file, tmp.r_r_payer);
		if (tmp.r_r_payer.length() == 0)
			continue;
		getline(file, tmp.r_r_recipient);
		file >> tmp.sum;
		cout << "| " << setw(3) << id
			<< "| " << setw(35) << tmp.r_r_payer
			<< "| " << setw(35) << tmp.r_r_recipient
			<< "| " << setw(10) << fixed << setprecision(2) << tmp.sum
			<< "|" << endl;
		id++;
	}
	cout << "============================================================================================" << endl;
}

void AddBill(fstream& file, Bill* new_bill) {
	file.clear();
	file.seekp(0, ios_base::end);
	file << new_bill->r_r_payer << endl
		<< new_bill->r_r_recipient << endl
		<< new_bill->sum << endl;
}

void main() {
	SetConsoleCP(1251); // встановлення сторінки win-cp1251 в потік вводу
	SetConsoleOutputCP(1251); // встановлення сторінки win-cp1251 в потік виводу
	cout.setf(ios_base::left);

	string f_name = cons::input<string>(nullptr, false, "Input filename: ");
	fstream stream(f_name);
	char message[256] = { 0 };
	unsigned int state = 0;

	if (!stream.is_open()) {
		stream.open(f_name, fstream::in | fstream::out | fstream::trunc);
		if (!stream.is_open()) {
			cout << "Can't open file!" << endl;
			return;
		}
	}
	do {
		cons::clear();
		draw_bills(stream);
		if (message[0] != '\0') {
			cout << message << endl;
			message[0] = '\0';
		}
		cout << "[1] Add bill" << endl
			<< "[2] Search bill by payer" << endl
			<< "[0] Exit" << endl;
		state = cons::input<unsigned int>(nullptr, false, "Select action: ");
		switch (state)
		{
		case 1: {
			Bill* tmp = InitBill();
			AddBill(stream, tmp);
			sprintf_s(message, 255, "Succesful add bill!");
		}
			break;
		case 2: {
			float sum;
			string payer = cons::input<string>(nullptr, false, "Input payer IBAN: ");
			if (sum = get_bill_by_payer(stream, payer), sum != -1.)
				sprintf_s(message, 255, "Succesful search bill - %s with sum %f!", payer, sum);
			else sprintf_s(message, 255, "Incorrect bill payer - %s!", payer.c_str());
			}
			  break;
		case 0:
			break;
		default:
			sprintf_s(message, 255, "Incorrect action!");
		}
	} while (state > 0);

	stream.close();
}