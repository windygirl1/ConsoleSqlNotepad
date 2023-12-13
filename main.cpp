#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>

#include "mysql_connection.h"
#include <cppconn/exception.h>
#include <cppconn/driver.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

// Variables to work with
const std::string SERVER = "tcp://localhost:*****";
const std::string USER = "*******";
const std::string PASSWORD = "*******";
const std::string NAMEOFDB = "notepad";
const std::string TABLENAME = "tableNotepad";

// Function to get the current time
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

	// Creating objects from the SQL library
	sql::Driver* driver;
	sql::Connection* conn = nullptr;
	sql::PreparedStatement* ptmt = nullptr;
	sql::Statement* stmt = nullptr;
	sql::ResultSet* res_set = nullptr;

	// Connecting to mySQL
	try {
		driver = get_driver_instance();
		conn = driver->connect(SERVER, USER, PASSWORD);
		std::cout << "Successful connection to mySQL." << "\n" << std::endl;
	}
	catch (sql::SQLException& ex) {
		if (conn != nullptr && !conn->isClosed()) {
			conn->close();
		}

		std::cout << "Connection error: " << ex.what() << "\n" << std::endl;
		system("pause");
		exit(1);
	}

	// Creating a database
	try {
		ptmt = conn->prepareStatement("CREATE DATABASE " + NAMEOFDB);
		ptmt->execute();

		std::cout << "The database has been successfully created." << "\n" << std::endl;
		if (ptmt != nullptr) {
			delete ptmt;
		}
	}
	catch (sql::SQLException& ex) {
		if (ex.getErrorCode() == 1007) {
			std::cout << "The database already exists." << "\n" << std::endl;
			if (ptmt != nullptr) {
				delete ptmt;
			}
		}
		else {
			std::cout << "Error creating database: " << ex.what() << "\n" << std::endl;
			if (ptmt != nullptr) {
				delete ptmt;
			}
			system("pause");
			exit(2);
		}
	}

	// Connecting to the database
	try {
		conn->setSchema(NAMEOFDB);
		std::cout << "Successful connection to the database." << "\n" << std::endl;
	}
	catch (sql::SQLException& ex) {
		std::cout << "Failed to connect to the database : " << ex.what() << "\n" << std::endl;
		system("pause");
		exit(3);
	}


	// Creating a table
	try {
		stmt = conn->createStatement();
		stmt->execute("CREATE TABLE " + TABLENAME + " ("
			"id INT AUTO_INCREMENT PRIMARY KEY, "
			"title VARCHAR(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci, "
			"text VARCHAR(500)  CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci, "
			"date VARCHAR(23)"
			") DEFAULT CHARACTER SET utf8mb4");

		std::cout << "The table has been created successfully." << "\n" << std::endl;
		if (stmt != nullptr) {
			delete stmt;
		}
	}
	catch (sql::SQLException& ex) {
		if (ex.getErrorCode() == 1050) {
			std::cout << "The table already exists." << "\n" << std::endl;
			if (stmt != nullptr) {
				delete stmt;
			}
		}
		else {
			std::cout << "Error creating table : " << ex.what() << "\n" << std::endl;
			if (stmt != nullptr) {
				delete stmt;
			}
			system("pause");
			exit(4);
		}
	}

	std::string command = "";
	std::string parametr = "";


	std::cout << "To view all commands, type help." << "\n" << std::endl;

	//The main cycle of the program
	while (true) {
		std::cout << "\n" << "> ";
		std::getline(std::cin, command);
		std::cout << "\n";

		std::stringstream ss(command);

		ss >> command >> parametr;

		bool isCommandKnow = false;

		if (command == "help" || command == "hp") {
			isCommandKnow = true;
			std::cout << "exit - Closing the program." << "\n" << std::endl;
			std::cout << "add - Add a new entry." << "\n" << std::endl;
			std::cout << "getposts - Get list of posts." << "\n" << std::endl;
			std::cout << "update - Update post in database. Must be used with a parameter to access a specific post. Example: 'update 1'." << "\n" << std::endl;
			std::cout << "post - Get post from database. Must be used with a parameter to access a specific post. Example: 'post 1'." << "\n" << std::endl;
			std::cout << "delete - Delete one specific post. Must be used with a parameter to access a specific post. Example: 'delete 1'." << "\n" << std::endl;
			std::cout << "deleteall - Remove all posts from the table. " << "\n" << std::endl;
		}

		if (command == "add") {
			isCommandKnow = true;
			std::string title;
			std::string text;

			std::cout << "Enter a title for the post: ";
			std::getline(std::cin, title);

			std::cout << "Enter post text: ";
			std::getline(std::cin, text);

			std::string currentTimeStr = getTime();

			try {
				ptmt = conn->prepareStatement("INSERT INTO " + TABLENAME + " (title, text, date) VALUES (?, ?, ?)");
				ptmt->setString(1, title);
				ptmt->setString(2, text);
				ptmt->setString(3, currentTimeStr);
				ptmt->execute();
				std::cout << "Post successfully created." << "\n" << std::endl;
				if (ptmt != nullptr) {
					delete ptmt;
				}
			}
			catch (sql::SQLException& ex) {
				std::cout << "Failed to add entry: " << ex.what() << "\n" << std::endl;
				if (ptmt != nullptr) {
					delete ptmt;
				}
			}
		}

		if (command == "getposts" || command == "gp") {
			isCommandKnow = true;
			try {
				stmt = conn->createStatement();
				res_set = stmt->executeQuery("SELECT * FROM " + TABLENAME);

				if (!res_set->next()) {
					std::cout << "You do not have any entries." << "\n" << std::endl;
				} else {
					res_set->beforeFirst();
					while (res_set->next()) {
						int id = res_set->getInt("id");
						std::string title = res_set->getString("title");
						std::string text = res_set->getString("text");
						std::string date = res_set->getString("date");

						std::cout << "ID: " << id << std::endl;
						std::cout << "-------------------------------" << "\n" << std::endl;
						std::cout << "Title: " << title << std::endl;
						std::cout << "-------------------------------" << "\n" << std::endl;
						std::cout << "Text: " << text << std::endl;
						std::cout << "-------------------------------" << "\n" << std::endl;
						std::cout << "Date: " << date << std::endl;
						std::cout << "-------------------------------" << "\n" << std::endl << std::endl;
					}
				}


				if (stmt != nullptr) {
					delete stmt;
				}

				if (res_set != nullptr) {
					delete res_set;
				}
			} catch (sql::SQLException& ex) {
				std::cout << "Failed to get list of posts: " << ex.what() << "\n" << std::endl;
				if (stmt != nullptr) {
					delete stmt;
				}

				if (res_set != nullptr) {
					delete res_set;
				}
			}
		}

		if (command == "update" && parametr != "" || command == "up" && parametr != "") {
			isCommandKnow = true;
			try {
				stmt = conn->createStatement();
				res_set = stmt->executeQuery("SELECT title, text FROM " + TABLENAME + " WHERE id = " + parametr);

				if (res_set->next()) {
					std::string title = res_set->getString("title");
					std::string text = res_set->getString("text");

					std::cout << "Your past values." << "\n" << std::endl;
					std::cout << "Title: " << title << "\n" << std::endl;
					std::cout << "Text: " << text << "\n" << std::endl;

					std::cout << "Enter a title for the post: ";
					std::getline(std::cin, title);

					std::cout << "Enter post text: ";
					std::getline(std::cin, text);

					try {
						stmt->execute("UPDATE " + TABLENAME + " SET title = '" + title + "', text = '" + text + "' WHERE id = " + parametr);
						std::cout << "Entry successfully updated." << "\n" << std::endl;
						if (stmt != nullptr) {
							delete stmt;
						}

						if (res_set != nullptr) {
							delete res_set;
						}
					} catch (sql::SQLException& ex) {
						std::cout << "Error updating post: " << ex.what() << "\n" << std::endl;
						if (stmt != nullptr) {
							delete stmt;
						}

						if (res_set != nullptr) {
							delete res_set;
						}
					}

				} else {
					std::cout << "Field with parameter '" << parametr << "' does not exist in the database." << std::endl;

					if (stmt != nullptr) {
						delete stmt;
					}

					if (res_set != nullptr) {
						delete res_set;
					}
				}
				
			}
			catch (sql::SQLException& ex) {
				std::cout << "Error while retrieving update data: " << ex.what() << "\n" << std::endl;
				if (res_set != nullptr) {
					delete res_set;
				}

				if (stmt != nullptr) {
					delete stmt;
				}
			}
		}

		if (command == "post" && parametr != "" || command == "p" && parametr != "") {
			isCommandKnow = true;
			try {
				stmt = conn->createStatement();
				res_set = stmt->executeQuery("SELECT * FROM " + TABLENAME + " WHERE id = " + parametr);

				
				if (res_set->next()) {
					int id = res_set->getInt("id");
					std::string title = res_set->getString("title");
					std::string text = res_set->getString("text");
					std::string date = res_set->getString("date");

					std::cout << "ID: " << id << std::endl;
					std::cout << "-------------------------------" << "\n" << std::endl;
					std::cout << "Title: " << title << std::endl;
					std::cout << "-------------------------------" << "\n" << std::endl;
					std::cout << "Text: " << text << std::endl;
					std::cout << "-------------------------------" << "\n" << std::endl;
					std::cout << "Date: " << date << std::endl;

					if (res_set != nullptr) {
						delete res_set;
					}

					if (stmt != nullptr) {
						delete stmt;
					}
				} else {
					std::cout << "Field with parameter '" << parametr << "' does not exist in the database." << std::endl;

					if (res_set != nullptr) {
						delete res_set;
					}

					if (stmt != nullptr) {
						delete stmt;
					}
				}

				
			} catch (sql::SQLException& ex) {
				std::cout << "Failed to get post: " << ex.what() << "\n" << std::endl;
				if (res_set != nullptr) {
					delete res_set;
				}

				if (stmt != nullptr) {
					delete stmt;
				}
			}
		}

		if (command == "delete" && parametr != "" || command == "dl" && parametr != "") {
			isCommandKnow = true;
			try {
				stmt = conn->createStatement();
				res_set = stmt->executeQuery("SELECT * FROM " + TABLENAME + " WHERE id = " + parametr);
				

				std::string areYouSure;
				
				if (res_set->next()) {
					std::cout << "Are you sure you want to delete this entry? Y/N" << "\n" << std::endl;
					int id = res_set->getInt("id");
					std::string title = res_set->getString("title");
					std::string text = res_set->getString("text");
					std::string date = res_set->getString("date");

					std::cout << "ID: " << id << std::endl;
					std::cout << "-------------------------------" << "\n" << std::endl;
					std::cout << "Title: " << title << std::endl;
					std::cout << "-------------------------------" << "\n" << std::endl;
					std::cout << "Text: " << text << std::endl;
					std::cout << "-------------------------------" << "\n" << std::endl;
					std::cout << "Date: " << date << std::endl;

					std::getline(std::cin, areYouSure);

					if (res_set != nullptr) {
						delete res_set;
					}

				}
				else {
					std::cout << "Field with parameter '" << parametr << "' does not exist in the database." << std::endl;

					if (res_set != nullptr) {
						delete res_set;
					}

					if (stmt != nullptr) {
						delete stmt;
					}
				}

				if (areYouSure == "Y") {
					try {
						stmt->execute("DELETE FROM " + TABLENAME + " WHERE id = " + parametr);
						std::cout << "Entry successfully deleted." << std::endl;
						if (stmt != nullptr) {
							delete stmt;
						}
					} catch (sql::SQLException& ex) {
						std::cout << "Failed to delete entry: " << "\n" << std::endl;
						if (stmt != nullptr) {
							delete stmt;
						}
					}
				} else {
					std::cout << "The entry was not deleted." << "\n" << std::endl;
					if (stmt != nullptr) {
						delete stmt;
					}
				}
			} catch (sql::SQLException& ex) {
				std::cout << "Post not found: " << ex.what() << "\n" << std::endl;

				if (res_set != nullptr) {
					delete res_set;
				}

				if (stmt != nullptr) {
					delete stmt;
				}
			}
		}

		if (command == "deleteall" || command == "dall") {
			isCommandKnow = true;
			std::string areYouSure;
			std::cout << "If you are sure you want to delete ALL entries, enter this: " << TABLENAME << "\n" << std::endl;
			std::getline(std::cin, areYouSure);

			if (areYouSure == TABLENAME) {
				try {
					stmt = conn->createStatement();
					stmt->execute("TRUNCATE " + TABLENAME);

					stmt->execute("ALTER TABLE " + TABLENAME + " AUTO_INCREMENT = 1");

					std::cout << "All entries were successfully deleted." << std::endl;

					if (stmt != nullptr) {
						delete stmt;
					}
				} catch (sql::SQLException& ex) {
					std::cout << "Failed to delete entries: " << ex.what() << "\n" << std::endl;
					if (stmt != nullptr) {
						delete stmt;
					}
				}
			} else {
				std::cout << "You entered the confirmation incorrectly: " << areYouSure << " != " << TABLENAME << "\n" << std::endl;
			}
		}

		if (command == "exit" || command == "ex") {
			isCommandKnow = true;
			if (res_set != nullptr) {
				delete res_set;
			}

			if (stmt != nullptr) {
				delete stmt;
			}

			if (ptmt != nullptr) {
				delete ptmt;
				ptmt = nullptr;
			}

			if (conn != nullptr) {
				conn->close();
				delete conn;
			}
			system("pause");
			exit(0);
		}

		if (!isCommandKnow) {
			std::cout << "Unknown command: " << command << std::endl;
		}
	}
	return 0;
}