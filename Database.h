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

// Класс базы данных. Хранит в себе информацию о пользовательских аккаунтах и преподавателях. Имеет указатель на свой интерфейс.
class Database {
private:
	// Класс аккаунта, хранит в себе Логин, Пароль, а также является ли учётная запись администраторской.
	struct Account {
		// Пароль
		string password = "";
		// Логин
		string login = "";
		// Является ли запись администраторской
		bool admin = false;

		// Конструктор класса аккаунта по умолчанию, ничего не задаёт.
		Account() {};
		// Конструктор класса аккаунта с параметрами, задаёт все поля.
		Account(string login, string password, bool admin) {
			this->login = login;
			this->password = password;
			this->admin = admin;
		}
		// Сравнить аккаунты.
		bool operator == (const Account& account) {
			return login == account.login && password == account.password;
		}
		// Дополнить файл информацией об аккаунте.
		void addToFile() const
		{
			fstream file("accounts.txt", ios::app);
			file << login << ' ' << Database::enDecrypt(password) << ' ' << admin << '\n';
			file.close();
		}
	};
	// Класс преподавателя, хранит в себе ФИО, дисциплину, стаж работы в годах, часы нагрузки, норму часов нагрузки. ///Будет дорабатываться
	struct Teacher {
		// ФИО
		string FIO = "";
		// Дисциплина
		string subject = "";
		// Квалификация
		string qualification = "";
		// Стаж работы , 0 - меньше года
		int yearsWork = 0;
		// Часы нагрузки
		int loadHours = 0;
		// Норма нагрузки
		int normalLoadHours = 0;

		// Конструктор класса преподавателя по умолчанию, ничего не задаёт.
		Teacher() {};
		// Конструктор класса преподавателя с параметрами, задаёт все поля.
		Teacher(string FIO, string subject, int yearsWork, int normalLoadHours, int loadHours) {
			this->FIO = FIO;
			this->subject = subject;
			this->yearsWork = yearsWork;
			this->normalLoadHours = normalLoadHours;
			this->loadHours = loadHours;
		}

		// Дополнить файл информацией об аккаунте.
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
	// База учётных записей
	vector<Account*> accounts;
	// База информации об учителях
	vector<Teacher*> teachers;

	// Текущая учётная запись
	Account* currentAccount = nullptr;
	// Указатель на экземпляр класса интерфейса базы данных
	DatabaseUI* ui;

	// Найти ID аккаунта с этим логином.
	int findID(string login) {
		for (int i = 0; i < accounts.size(); i++) {
			if (accounts.at(i)->login == login) return i;
		}
		return -1;
	}
	// Проверить пароль аккаунта.
	const bool checkPassword(int id, string password) {
		return accounts.at(id)->password == password;
	}
	// Шифрует/расшифровывает строку
	static string enDecrypt(string text) {
		char key = text.size() % 10;

		for (char& ch : text)
			ch ^= key;
		return text;
	}

public:
	// Конструктор класса базы данных по умолчанию, создаёт и запускает интерфейс.
	Database();
	// Деструктор класса базы данных, очищает память от интерфейса и записывает данные в файлы.
	~Database();
	// Является ли пользователь администратором
	const bool isAdmin() const { return currentAccount->admin; }
	// Зашёл ли пользователь в аккаунт.
	const bool isLoggedIn() const { return currentAccount; }
	// Есть ли аккаунт с таким логином.
	const bool checkLogin(string login) {
		for (Account* ac : accounts) {
			if (ac->login == login) return true;
		}
		return false;
	}
	// Получить логин текущего аккаунта.
	const string getLogin() const { return currentAccount->login; }
	// Авторизация пользователя.
	void login();
	// Деавторизация пользователя.
	void deauthorize() { currentAccount = nullptr; }
	// Редактировать данные об преподавателях //

	// Показать преподавателей.
	void showTeachers();
	// Добавить преподавателя.
	void addTeacher();
	// Удалить преподавателя.
	void removeTeacher();

	// Редактировать учётные записи //
	void showAccounts();
	// Создание учётной записи.
	void addAccount();
	// Удаление учётной записи.
	void removeAccount();
	// Содержит ли вектор, значение
	template<typename T>
	bool contains(vector<T> vec, T value) {
		for (T t : vec) {
			if (t == value) return true;
		}
		return false;
	}
	// Балансировка нагрузки учителей ???
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
	// Запись учителей в файл.
	void writeTeachersToFile()
	{
		fstream file("teachers.txt", ios::out);
		file << teachers.size() << '\n';
		file.close();

		for (int i = 0; i < teachers.size(); i++)
			teachers.at(i)->addToFile();
	}
	// Чтение учителей из файла.
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

	// Запись аккаунтов в файл.
	void writeAccountsToFile()
	{
		fstream file("accounts.txt", ios::out);
		file << accounts.size() << '\n';
		file.close();

		for (int i = 0; i < accounts.size(); i++)
			accounts.at(i)->addToFile();
	}
	// Чтение аккаунтов из файла.
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

// Класс интерфейса. Имеет методы ввода-вывода информации, общие для всех интерфейсов.
class UI {

public:
	// Цвета 0-Белый,1-Красный,2-Синий,3-Зелёный,4-Оранжевый,5-Розовый,6-Жёлтый.
	enum colors : int {
		White,
		Red,
		Blue,
		Green,
		Orange,
		Pink,
		Yellow
	};
	// Задать цвет текста выводимого в консоль.
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
	// Ввести переменную любого типа выведя перед этим строку и двоеточие.
	template<typename T>
	T input(string text, bool newLine = true) {
		T input{};
		while (true) {
			printColor(text + " : ", newLine);
			cin >> input;
			if (cin.fail()) {
				printColor("&1Неправильный тип данных!");
				cin.clear();
				cin.ignore();
			}
			else {
				return input;
			}
		}
	}
	// Ввести переменную любого типа в диапозоне. Или, если min и max равны, ввести переменную от min.
	template<typename T>
	T inputRange(string text, T min, T max, bool newLine = true) {
		T input{};
		while (true) {
			printColor(text + " : ", newLine);
			cin >> input;
			if (cin.fail()) {
				printColor("&1Неправильный тип данных!");
				cin.clear();
				cin.ignore();
			}
			else if (((max != min)) && (input > max || input < min)) {
				printColor("&1Введите значение в диапазоне от " + to_string(min) + " до " + to_string(max));
			}
			else if (max == min && input < min) {
				printColor("&1Введите значение от " + to_string(min));
			}
			else {
				return input;
			}
		}
	}
	// Вывести переменную любого типа.
	template <typename T>
	void print(T data, bool newLine = true) {
		if (newLine)  cout << '\n';
		cout << data;
	}
	// Вывести строку с цветным форматированием. Пример: "Мой &2текст &4цветной".
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
	// Ждать любого ввода.
	void pressAnyButton() {
		cout << '\n';
		system("pause");
	}
	// Диалог подтверждения.
	const bool confirm() {
		return input<bool>("&6Вы уверены? (&10-Нет&0,&31-Да&0)");
	}
};
// Класс интерфейса базы данных, созданный для работы с базами пользовательских аккаунтов и преподавателей.
class DatabaseUI : public UI {
private:
	// Указатель на экземпляр класса базы данных, которому принадлежит интерфейс
	Database* database;
public:
	// Конструктор интерфейса для базы данных.
	DatabaseUI(Database* database) {
		setlocale(LC_ALL, "Russian");
		this->database = database;
	}
	/// Заготовленные менюшки

	// Начало работы интерфейса, основной цикл программы.
	void startUI() {
		while (true) {
			system("cls");

			database->login();
			if (database->isAdmin()) adminMenu();
			else userMenu();

			bool choice = input<bool>("Желаете &1выйти&0 или &3продолжить &0использование программы? (&10-Выйти&0/&31-Продолжить&0)");

			if (!choice) {
				if (!confirm()) continue; /// XD
				printColor("&6Хорошего дня!");
				return;
			}
		}
	}
	// Меню для гостя.
	void userMenu() {
		while (true) {
			if (!database->isLoggedIn()) return;
			system("cls");
			printColor("&6Добро пожаловать " + database->getLogin());
			printColor("1 - Просмотреть список преподавателей");
			printColor("0 - Выйти");
			int choice = inputRange<int>("Выберите действие из списка", 0, 1);
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
	// Меню для администратора.
	void adminMenu() {
		while (true) {
			if (!database->isLoggedIn()) return;
			system("cls");
			printColor("&6Добро пожаловать " + database->getLogin());
			printColor("1 - Редактировать список учителей");
			printColor("2 - Редактировать список аккаунтов");
			printColor("0 - Выйти");
			int choice = inputRange<int>("Выберите действие из списка", 0, 2);
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

	// Меню для редактирования списка преподавателей.
	void teachersEditing() {
		while (true) {
			if (!database->isLoggedIn()) return;
			system("cls");
			printColor("Редактировать список преподавателей");
			printColor("1 - Добавить преподавателя");
			printColor("2 - Удалить преподавателя");
			printColor("3 - Посмотреть список преподавателей");
			printColor("4 - Сбалансировать нагрузку у преподавателей");
			printColor("0 - Выйти");
			int choice = inputRange<int>("Выберите действие из списка", 0, 4);
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
	// Меню для редактирования списка аккаунтов.
	void accountsEditing() {
		while (true) {
			if (!database->isLoggedIn()) return;
			system("cls");
			printColor("Редактировать список аккаунтов");
			printColor("1 - Добавить аккаунт");
			printColor("2 - Удалить аккаунт");
			printColor("3 - Посмотреть список аккаунтов");
			printColor("0 - Выйти");
			int choice = inputRange<int>("Выберите действие из списка", 0, 3);
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

// Определения методов\переменных

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
			// Вывод ошибки
			ui->printColor("&1Пользователь уже авторизован");
			return;
		}
		readAccountsFromFile();
		if (accounts.empty())
		{
			// Вывод ошибки
			ui->printColor("&1Нет аккаунтов для авторизации");
			ui->printColor("Создайте аккаунт для авторизации");
			addAccount();
			currentAccount = accounts.at(0);
			ui->printColor("&3Авторизация успешна");
			return;
		}
		ui->printColor("Войдите в аккаунт");
		string login;
		do {
			login = ui->input<string>("Введите логин");
			if (checkLogin(login)) {
				break;
			}
			ui->printColor("&1Неверный логин!");
		} while (true);
		int accountID = findID(login);
		int attempts = 3;
		string password;
		do {
			password = ui->input<string>("Введите пароль");
			if (checkPassword(accountID, password) || attempts-- < 0) {
				break;
			}
			ui->printColor("&1Неверный пароль!");
		} while (true);

		if (attempts < 0) {
			ui->printColor("&1Вы неверно ввели пароль больше 3 раз, попробуйте снова.");
			continue;
		}

		currentAccount = accounts.at(accountID);
		ui->printColor("&3Авторизация успешна");

		return;
	}
}

void Database::showTeachers() {
	if (teachers.empty())
	{
		ui->printColor("&1Список пуст");
		return;
	}
	for (int i = 0; i < teachers.size(); i++) {
		ui->printColor("\nID: " + to_string(i) + "\tФИО: " + teachers.at(i)->FIO + "\n\tПредмет: " + teachers.at(i)->subject + "\n\tТекущие часы нагрузки: " + to_string(teachers.at(i)->loadHours) + "\tНорма часов нагрузки: " + to_string(teachers.at(i)->normalLoadHours));///
	}
}

void Database::addTeacher()
{
	string FIO = ui->input<string>("Введите ФИО");
	string subject = ui->input<string>("Введите дисциплину");
	int yearsWork = ui->inputRange<int>("Введите стаж работы",0,0);
	int normalLoadHours = ui->inputRange<int>("Введите норму часов нагрузки",0,0);
	int loadHours = ui->inputRange<int>("Введите часы нагрузки",0,0);

	teachers.emplace_back(new Teacher(FIO, subject, yearsWork, normalLoadHours, loadHours));

	writeTeachersToFile();
}

void Database::removeTeacher() {
	showTeachers();

	if (teachers.empty()) {
		ui->printColor("&1 Нечего удалять");
		return;
	}
	int id = ui->inputRange<int>("Выберите номер удаляемого преподавателя", 0, teachers.size() - 1);

	if (!ui->confirm()) return;

	delete teachers.at(id);
	teachers.erase(teachers.begin() + id);

	writeTeachersToFile();

	ui->printColor("&3Преподаватель успешно удалён");
}

void Database::showAccounts() {
	if (accounts.empty())
	{
		ui->printColor("&1Список пуст");
		return;
	}
	for (int i = 0; i < accounts.size(); i++) {
		ui->printColor(to_string(i) + " " + accounts.at(i)->login);
		if (accounts.at(i)->admin) ui->printColor(" администратор", false);
		else ui->printColor(" гость", false);
	}
}

void Database::addAccount() {
	string login;
	while (true) {
		login = ui->input<string>("Введите логин");
		if (!checkLogin(login)) break;
		ui->printColor("&1Этот пользователь уже существует");
	}
	string password = ui->input<string>("Введите пароль");
	bool admin = ui->input<bool>("Это администратор? (&10-Нет&0,&31-Да&0)");
	accounts.emplace_back(new Account(login, password, admin));

	writeAccountsToFile();
}

void Database::removeAccount() {
	showAccounts();

	if (accounts.empty()) {
		ui->printColor("&1 Нечего удалять");
		return;
	}
	int id = ui->inputRange<int>("Выберите номер удаляемого аккаунта", 0, accounts.size() - 1);

	if (!ui->confirm()) return;

	if (currentAccount == accounts.at(id)) {
		system("cls");
		ui->printColor("&1Вы удалили аккаунт, в котором были авторизованы");
		deauthorize();
	}

	delete accounts.at(id);
	accounts.erase(accounts.begin() + id);

	writeAccountsToFile();

	ui->printColor("&3Аккаунт успешно удалён");
}