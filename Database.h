#pragma once

#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include "windows.h"

using namespace std;

class DatabaseUI;

// ����� ���� ������. ������ � ���� ���������� � ���������������� ��������� � ��������������. ����� ��������� �� ���� ���������.
class Database {
private:
	// ����� ��������, ������ � ���� �����, ������, � ����� �������� �� ������� ������ �����������������.
	struct Account {
		// ������
		string password = "";
		// �����
		string login = "";
		// �������� �� ������ �����������������
		bool admin = false;

		// ����������� ������ �������� �� ���������, ������ �� �����.
		Account() {};
		// ����������� ������ �������� � �����������, ����� ��� ����.
		Account(string login, string password, bool admin) {
			this->login = login;
			this->password = password;
			this->admin = admin;
		}
		// �������� ��������.
		bool operator == (const Account& account) {
			return login == account.login && password == account.password;
		}
		// ��������� ���� ����������� �� ��������.
		void addToFile() const
		{
			fstream file("accounts.txt", ios::app);
			file << login << ' ' << Database::enDecrypt(password) << ' ' << admin << '\n';
			file.close();
		}
	};
	// ����� �������������, ������ � ���� ���, ����������, ���� ������ � �����, ���� ��������, ����� ����� ��������. ///����� ��������������
	struct Teacher {
		// ���
		string FIO = "";
		// ����������
		string subject = "";
		// ������������
		string qualification = "";
		// ���� ������ , 0 - ������ ����
		int yearsWork = 0;
		// ���� ��������
		int loadHours = 0;
		// ����� ��������
		int normalLoadHours = 0;

		// ����������� ������ ������������� �� ���������, ������ �� �����.
		Teacher() {};
		// ����������� ������ ������������� � �����������, ����� ��� ����.
		Teacher(string FIO, string subject, int yearsWork, int normalLoadHours, int loadHours) {
			this->FIO = FIO;
			this->subject = subject;
			this->yearsWork = yearsWork;
			this->normalLoadHours = normalLoadHours;
			this->loadHours = loadHours;
		}

		// ��������� ���� ����������� �� ��������.
		void addToFile() const
		{
			fstream file("teachers.txt", ios::app);
			file << FIO << ' ' << subject << ' ' << yearsWork << ' ' << normalLoadHours << ' ' << loadHours << '\n';
			file.close();
		}

		bool operator<(const Teacher& th) {
			return yearsWork < th.yearsWork;
		}
	};
	// ���� ������� �������
	vector<Account*> accounts;
	// ���� ���������� �� ��������
	vector<Teacher*> teachers;

	// ������� ������� ������
	Account* currentAccount = nullptr;
	// ��������� �� ��������� ������ ���������� ���� ������
	DatabaseUI* ui;

	// ����� ID �������� � ���� �������.
	int findID(string login) {
		for (int i = 0; i < accounts.size(); i++) {
			if (accounts.at(i)->login == login) return i;
		}
		return -1;
	}
	// ��������� ������ ��������.
	const bool checkPassword(int id, string password) {
		return accounts.at(id)->password == password;
	}
	// �������/�������������� ������
	static string enDecrypt(string text) {
		char key = text.size() % 10;

		for (char& ch : text)
			ch ^= key;
		return text;
	}

public:
	// ����������� ������ ���� ������ �� ���������, ������ � ��������� ���������.
	Database();
	// ���������� ������ ���� ������, ������� ������ �� ���������� � ���������� ������ � �����.
	~Database();
	// �������� �� ������������ ���������������
	const bool isAdmin() const { return currentAccount->admin; }
	// ����� �� ������������ � �������.
	const bool isLoggedIn() const { return currentAccount; }
	// ���� �� ������� � ����� �������.
	const bool checkLogin(string login) {
		for (Account* ac : accounts) {
			if (ac->login == login) return true;
		}
		return false;
	}
	// �������� ����� �������� ��������.
	const string getLogin() const { return currentAccount->login; }
	// ����������� ������������.
	void login();
	// ������������� ������������.
	void deauthorize() { currentAccount = nullptr; }
	// ������������� ������ �� �������������� //

	// �������� ��������������.
	void showTeachers();
	// �������� �������������.
	void addTeacher();
	// ������� �������������.
	void removeTeacher();

	// ������������� ������� ������ //
	void showAccounts();
	// �������� ������� ������.
	void addAccount();
	// �������� ������� ������.
	void removeAccount();
	// �������� �� ������, ��������
	template<typename T>
	bool contains(vector<T> vec, T value) {
		for (T t : vec) {
			if (t == value) return true;
		}
		return false;
	}
	// ������������ �������� �������� ???
	void balanceTeachers() {
		map<string, vector<Teacher*>> teachersBySubjects;
		vector<string> subjects;
		for (Teacher* th : teachers) {
			teachersBySubjects[th->subject].push_back(th);
			if (!contains(subjects, th->subject)) subjects.push_back(th->subject);
		}
		for (int i = 0; i < subjects.size(); i++) {
			int hoursLeft = 0;

			sort(teachersBySubjects.at(subjects.at(i)).begin(), teachersBySubjects.at(subjects.at(i)).end());

			for (Teacher* th : teachersBySubjects[subjects.at(i)]) {
				hoursLeft += th->loadHours - th->normalLoadHours;
				th->loadHours = th->normalLoadHours;
			}
			for (Teacher* th : teachersBySubjects[subjects.at(i)]) th->loadHours += hoursLeft / teachersBySubjects[subjects.at(i)].size();
		}
	}
	// ������ �������� � ����.
	void writeTeachersToFile()
	{
		fstream file("teachers.txt", ios::out);
		file << teachers.size() << '\n';
		file.close();

		for (int i = 0; i < teachers.size(); i++)
			teachers.at(i)->addToFile();
	}
	// ������ �������� �� �����.
	void readTeachersFromFile()
	{
		teachers.clear();
		fstream file("teachers.txt", ios::in);

		if (file.eof() || !file.is_open())
		{
			file.close();
			return;
		}

		string temp;

		getline(file, temp, '\n');
		int size = stoi(temp);

		for (int i = 0; i < size; i++)
		{
			string FIO, subject;
			int yearsWork, normalLoadHours, loadHours;
			getline(file, FIO, ' ');
			getline(file, subject, ' ');
			getline(file, temp, ' ');
			yearsWork = stoi(temp);
			getline(file, temp, ' ');
			normalLoadHours = stoi(temp);
			getline(file, temp, '\n');
			loadHours = stoi(temp);

			teachers.emplace_back(new Teacher(FIO, subject, yearsWork, normalLoadHours, loadHours));
		}
		file.close();
	}

	// ������ ��������� � ����.
	void writeAccountsToFile()
	{
		fstream file("accounts.txt", ios::out);
		file << accounts.size() << '\n';
		file.close();

		for (int i = 0; i < accounts.size(); i++)
			accounts.at(i)->addToFile();
	}
	// ������ ��������� �� �����.
	void readAccountsFromFile()
	{
		accounts.clear();
		fstream file("accounts.txt", ios::in);

		if (file.eof() || !file.is_open())
		{
			file.close();
			return;
		}

		string temp;

		getline(file, temp, '\n');
		int size = stoi(temp);

		for (int i = 0; i < size; i++)
		{
			string login, password;
			getline(file, login, ' ');
			getline(file, password, ' ');

			//encodeOrDecodePassword(password);
			getline(file, temp, '\n');
			bool isAdmin = stoi(temp);

			accounts.emplace_back(new Account(login, enDecrypt(password), isAdmin));
		}
		file.close();
	}

};

// ����� ����������. ����� ������ �����-������ ����������, ����� ��� ���� �����������.
class UI {

public:
	// ����� 0-�����,1-�������,2-�����,3-������,4-���������,5-�������,6-Ƹ����.
	enum colors : int {
		White,
		Red,
		Blue,
		Green,
		Orange,
		Pink,
		Yellow
	};
	// ������ ���� ������ ���������� � �������.
	void setColor(UI::colors color) {
		HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		switch (color) {
		case UI::colors::Red:
			SetConsoleTextAttribute(handle, 12);
			break;
		case UI::colors::Green:
			SetConsoleTextAttribute(handle, 10);
			break;
		case UI::colors::Blue:
			SetConsoleTextAttribute(handle, 9);
			break;
		case UI::colors::Yellow:
			SetConsoleTextAttribute(handle, 14);
			break;
		case UI::colors::Orange:
			SetConsoleTextAttribute(handle, 6);
			break;
		case UI::colors::Pink:
			SetConsoleTextAttribute(handle, 13);
			break;
		default:
			SetConsoleTextAttribute(handle, 7);
			break;
		}
	}
	// ������ ���������� ������ ���� ������ ����� ���� ������ � ���������.
	template<typename T>
	T input(string text, bool newLine = true) {
		T input{};
		while (true) {
			printColor(text + " : ", newLine);
			cin >> input;
			if (cin.fail()) {
				printColor("&1������������ ��� ������!");
				cin.clear();
				cin.ignore();
			}
			else {
				return input;
			}
		}
	}
	// ������ ���������� ������ ���� � ���������. ���, ���� min � max �����, ������ ���������� �� min.
	template<typename T>
	T inputRange(string text, T min, T max, bool newLine = true) {
		T input{};
		while (true) {
			printColor(text + " : ", newLine);
			cin >> input;
			if (cin.fail()) {
				printColor("&1������������ ��� ������!");
				cin.clear();
				cin.ignore();
			}
			else if (((max != min)) && (input > max || input < min)) {
				printColor("&1������� �������� � ��������� �� " + to_string(min) + " �� " + to_string(max));
			}
			else if (max == min && input < min) {
				printColor("&1������� �������� �� " + to_string(min));
			}
			else {
				return input;
			}
		}
	}
	// ������� ���������� ������ ����.
	template <typename T>
	void print(T data, bool newLine = true) {
		if (newLine)  cout << '\n';
		cout << data;
	}
	// ������� ������ � ������� ���������������. ������: "��� &2����� &4�������".
	void printColor(string str, bool newLine = true) {
		if (newLine)  cout << '\n';
		bool flag = false;
		for (char ch : str) {
			if (!flag)
				if (ch != '&')
					cout << ch;
				else flag = true;
			else {
				setColor(static_cast<UI::colors>((ch - 48)));
				flag = false;
			}
		}
		setColor(White);
	}
	// ����� ������ �����.
	void pressAnyButton() {
		cout << '\n';
		system("pause");
	}
	// ������ �������������.
	const bool confirm() {
		return input<bool>("&6�� �������? (&10-���&0,&31-��&0)");
	}
};
// ����� ���������� ���� ������, ��������� ��� ������ � ������ ���������������� ��������� � ��������������.
class DatabaseUI : public UI {
private:
	// ��������� �� ��������� ������ ���� ������, �������� ����������� ���������
	Database* database;
public:
	// ����������� ���������� ��� ���� ������.
	DatabaseUI(Database* database) {
		setlocale(LC_ALL, "Russian");
		this->database = database;
	}
	/// ������������� �������

	// ������ ������ ����������, �������� ���� ���������.
	void startUI() {
		while (true) {
			system("cls");

			database->login();
			if (database->isAdmin()) adminMenu();
			else userMenu();

			bool choice = input<bool>("������� &1�����&0 ��� &3���������� &0������������� ���������? (&10-�����&0/&31-����������&0)");

			if (!choice) {
				if (!confirm()) continue; /// XD
				printColor("&6�������� ���!");
				return;
			}
		}
	}
	// ���� ��� �����.
	void userMenu() {
		while (true) {
			if (!database->isLoggedIn()) return;
			system("cls");
			printColor("&6����� ���������� " + database->getLogin());
			printColor("1 - ����������� ������ ��������������");
			printColor("0 - �����");
			int choice = inputRange<int>("�������� �������� �� ������", 0, 1);
			system("cls");
			switch (choice) {
			case 0:
				database->deauthorize();
				return;
				break;
			case 1:
				database->showTeachers();
				pressAnyButton();
				break;
			}
		}
	}
	// ���� ��� ��������������.
	void adminMenu() {
		while (true) {
			if (!database->isLoggedIn()) return;
			system("cls");
			printColor("&6����� ���������� " + database->getLogin());
			printColor("1 - ������������� ������ ��������");
			printColor("2 - ������������� ������ ���������");
			printColor("0 - �����");
			int choice = inputRange<int>("�������� �������� �� ������", 0, 2);
			system("cls");
			switch (choice) {
			case 0:
				database->deauthorize();
				return;
				break;
			case 1:
				teachersEditing();
				break;
			case 2:
				accountsEditing();
				break;
			}
		}
	}

	// ���� ��� �������������� ������ ��������������.
	void teachersEditing() {
		while (true) {
			if (!database->isLoggedIn()) return;
			system("cls");
			printColor("������������� ������ ��������������");
			printColor("1 - �������� �������������");
			printColor("2 - ������� �������������");
			printColor("3 - ���������� ������ ��������������");
			printColor("4 - �������������� �������� � ��������������");
			printColor("0 - �����");
			int choice = inputRange<int>("�������� �������� �� ������", 0, 4);
			system("cls");
			switch (choice) {
			case 0:
				return;
				break;
			case 1:
				database->addTeacher();
				break;
			case 2:
				database->removeTeacher();
				break;
			case 3:
				system("cls");
				database->showTeachers();
				pressAnyButton();
				break;
			case 4:
				database->balanceTeachers();
				break;
			}
		}
	}
	// ���� ��� �������������� ������ ���������.
	void accountsEditing() {
		while (true) {
			if (!database->isLoggedIn()) return;
			system("cls");
			printColor("������������� ������ ���������");
			printColor("1 - �������� �������");
			printColor("2 - ������� �������");
			printColor("3 - ���������� ������ ���������");
			printColor("0 - �����");
			int choice = inputRange<int>("�������� �������� �� ������", 0, 3);
			system("cls");
			switch (choice) {
			case 0:
				return;
				break;
			case 1:
				database->addAccount();
				break;
			case 2:
				database->removeAccount();
				break;
			case 3:
				system("cls");
				database->showAccounts();
				pressAnyButton();
				break;
			}
		}
	}
};

// ����������� �������\����������

Database::Database() {
	readTeachersFromFile();

	ui = new DatabaseUI(this);
	ui->startUI();
}

Database::~Database() {
	writeAccountsToFile();
	writeTeachersToFile();
	delete ui;
}

void Database::login() {
	while (true) {
		if (currentAccount) {
			// ����� ������
			ui->printColor("&1������������ ��� �����������");
			return;
		}
		readAccountsFromFile();
		if (accounts.empty())
		{
			// ����� ������
			ui->printColor("&1��� ��������� ��� �����������");
			ui->printColor("�������� ������� ��� �����������");
			addAccount();
			currentAccount = accounts.at(0);
			ui->printColor("&3����������� �������");
			return;
		}
		ui->printColor("������� � �������");
		string login;
		do {
			login = ui->input<string>("������� �����");
			if (checkLogin(login)) {
				break;
			}
			ui->printColor("&1�������� �����!");
		} while (true);
		int accountID = findID(login);
		int attempts = 3;
		string password;
		do {
			password = ui->input<string>("������� ������");
			if (checkPassword(accountID, password) || attempts-- < 0) {
				break;
			}
			ui->printColor("&1�������� ������!");
		} while (true);

		if (attempts < 0) {
			ui->printColor("&1�� ������� ����� ������ ������ 3 ���, ���������� �����.");
			continue;
		}

		currentAccount = accounts.at(accountID);
		ui->printColor("&3����������� �������");

		return;
	}
}

void Database::showTeachers() {
	if (teachers.empty())
	{
		ui->printColor("&1������ ����");
		return;
	}
	for (int i = 0; i < teachers.size(); i++) {
		ui->printColor("\nID: " + to_string(i) + "\t���: " + teachers.at(i)->FIO + "\n\t�������: " + teachers.at(i)->subject + "\n\t������� ���� ��������: " + to_string(teachers.at(i)->loadHours) + "\t����� ����� ��������: " + to_string(teachers.at(i)->normalLoadHours));///
	}
}

void Database::addTeacher()
{
	string FIO = ui->input<string>("������� ���");
	string subject = ui->input<string>("������� ����������");
	int yearsWork = ui->inputRange<int>("������� ���� ������",0,0);
	int normalLoadHours = ui->inputRange<int>("������� ����� ����� ��������",0,0);
	int loadHours = ui->inputRange<int>("������� ���� ��������",0,0);

	teachers.emplace_back(new Teacher(FIO, subject, yearsWork, normalLoadHours, loadHours));

	writeTeachersToFile();
}

void Database::removeTeacher() {
	showTeachers();

	if (teachers.empty()) {
		ui->printColor("&1 ������ �������");
		return;
	}
	int id = ui->inputRange<int>("�������� ����� ���������� �������������", 0, teachers.size() - 1);

	if (!ui->confirm()) return;

	delete teachers.at(id);
	teachers.erase(teachers.begin() + id);

	writeTeachersToFile();

	ui->printColor("&3������������� ������� �����");
}

void Database::showAccounts() {
	if (accounts.empty())
	{
		ui->printColor("&1������ ����");
		return;
	}
	for (int i = 0; i < accounts.size(); i++) {
		ui->printColor(to_string(i) + " " + accounts.at(i)->login);
		if (accounts.at(i)->admin) ui->printColor(" �������������", false);
		else ui->printColor(" �����", false);
	}
}

void Database::addAccount() {
	string login;
	while (true) {
		login = ui->input<string>("������� �����");
		if (!checkLogin(login)) break;
		ui->printColor("&1���� ������������ ��� ����������");
	}
	string password = ui->input<string>("������� ������");
	bool admin = ui->input<bool>("��� �������������? (&10-���&0,&31-��&0)");
	accounts.emplace_back(new Account(login, password, admin));

	writeAccountsToFile();
}

void Database::removeAccount() {
	showAccounts();

	if (accounts.empty()) {
		ui->printColor("&1 ������ �������");
		return;
	}
	int id = ui->inputRange<int>("�������� ����� ���������� ��������", 0, accounts.size() - 1);

	if (!ui->confirm()) return;

	if (currentAccount == accounts.at(id)) {
		system("cls");
		ui->printColor("&1�� ������� �������, � ������� ���� ������������");
		deauthorize();
	}

	delete accounts.at(id);
	accounts.erase(accounts.begin() + id);

	writeAccountsToFile();

	ui->printColor("&3������� ������� �����");
}