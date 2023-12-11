#include <iostream>
#include <chrono>
#include <ctime>

#include "mysql_connection.h"
#include <cppconn/exception.h>
#include <cppconn/driver.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

// Переменные для работы
const std::string SERVER = "tcp://localhost:3306";
const std::string USER = "root";
const std::string PASSWORD = "";
const std::string NAMEOFDB = "notepad";
const std::string TABLENAME = "tableNotepad";

// Функция для получения текущего времени
std::string getTime() {
	auto currentTime = std::chrono::system_clock::now();
	std::time_t currentTime_t = std::chrono::system_clock::to_time_t(currentTime);

	std::tm currentTime_tm;
	#if defined(_WIN32)
	localtime_s(&currentTime_tm, &currentTime_t);
	#else
	localtime_r(&currentTime_t, &currentTime_tm);
	#endif

	char currentTimeStr[26];
	std::strftime(currentTimeStr, sizeof(currentTimeStr), "%c", &currentTime_tm);

	return currentTimeStr;
}

int main() {
	setlocale(LC_ALL, "RU");

	// Создание объектов из библиотеки SQL
	sql::Driver* driver;
	sql::Connection* conn = nullptr;
	sql::ResultSet* res_set;

	// Подключение к mySQL
	try {
		driver = get_driver_instance();
		conn = driver->connect(SERVER, USER, PASSWORD);
	} catch (sql::SQLException& ex) {
		if (conn != nullptr && !conn->isClosed()) {
			conn->close();
		}

		std::cout << "Ошибка подключения: " << ex.what() << std::endl;
		system("pause");
		exit(1);
	}

	// Создание БД
	try {
		sql::PreparedStatement* ptmt = nullptr;

		ptmt = conn->prepareStatement("CREATE DATABASE " + NAMEOFDB);
		conn->prepareStatement("SET NAMES 'utf8mb4'");
		ptmt->execute();

		std::cout << "База данных успешно создана." << std::endl;

		if (ptmt != nullptr) {
			delete ptmt;
		}
	} catch (sql::SQLException& ex) {
		if (ex.getErrorCode() == 1007) {
			std::cout << "База данных уже существует." << std::endl;
		} else {
			std::cout << "Ошибка создания БД: " << ex.what() << std::endl;
			system("pause");
			exit(2);
		}
	}

	// Подключение к БД
	try {
		conn->setSchema(NAMEOFDB);
		std::cout << "Успешное подключение к БД." << std::endl;
	} catch (sql::SQLException& ex) {
		std::cout << "Не удалось подключится к БД: " << ex.what() << std::endl;
		system("pause");
		exit(3);
	}

	
	// создание таблицы
	try {
		sql::Statement* stmt;

		stmt = conn->createStatement();
		stmt->execute("CREATE TABLE " + TABLENAME + " ("
			"id INT AUTO_INCREMENT PRIMARY KEY, "
			"title VARCHAR(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci, "
			"text VARCHAR(500)  CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci, "
			"date VARCHAR(23)"
			") DEFAULT CHARACTER SET utf8mb4");

		std::cout << "Таблица успешно создана" << std::endl;
		if (stmt != nullptr) {
			delete stmt;
		}
	} catch (sql::SQLException& ex) {
		if (ex.getErrorCode() == 1050) {
			std::cout << "Таблица уже существует." << std::endl;
		} else {
			std::cout << "Ошибка при создании таблицы: " << ex.what() << std::endl;
			system("pause");
			exit(4);
		}
	}

	std::string command = "";


	std::cout << "Для простмотра всех команд введите help." << std::endl;

	while (true) {
		std::cout << "> "; 
		std::getline(std::cin, command);

		if (command == "help" || command == "hp") {
			std::cout << "exit - закрытие программы." << std::endl;
			std::cout << "add - добавить новую запись." << std::endl;
		}

		if (command == "add") {
			std::string title;
			std::string text;

			std::cout << "Введите заголовок для записи: ";
			std::getline(std::cin, title);

			std::cout << "Введите текст записи: ";
			std::getline(std::cin, text);

			std::string currentTimeStr = getTime();

			try {
				sql::PreparedStatement* ptmt = nullptr;
				ptmt = conn->prepareStatement("INSERT INTO " + TABLENAME + " (title, text, date) VALUES (?, ?, ?)");
				ptmt->setString(1, title);
				ptmt->setString(2, text);
				ptmt->setString(3, currentTimeStr);
				conn->setClientOption("MYSQL_SET_CHARSET_NAME", "utf8mb4_general_ci");
				ptmt->execute();
				if (ptmt != nullptr) {
					delete ptmt;
				}
			} catch (sql::SQLException& ex) {
				std::cout << "Не удалось добавить запись: " << ex.what() << std::endl;
			}
		}

		if (command == "exit" || command == "ex") {
			system("pause");
			exit(0);
		}
	}

	

	if (conn != nullptr) {
		conn->close();
		delete conn;
	}

	return 0;
}