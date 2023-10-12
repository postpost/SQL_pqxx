#include <iostream>
#include <pqxx/pqxx>
#include <Windows.h>
#pragma execution_character_set("utf-8")

class ClientDB 
{
public:
	//ћетод, создающий структуру Ѕƒ (таблицы)
	void CreateTable(pqxx::connection& conn) 
	{
		pqxx::transaction tx(conn);
		tx.exec("CREATE TABLE IF NOT EXISTS ClientDB "
			"(id SERIAL primary key, "
			"firstName TEXT not null, "
			"lastName TEXT not null, "
			"email TEXT not null, "
			"CONSTRAINT proper_email "
			"CHECK (email ~* '^[A-Za-z0-9._+%-]+@[A-Za-z0-9.-]+[.][A-Za-z]+$'))"); //citext not null UNIQUE
		tx.commit();
		pqxx::transaction tx01(conn);
		tx01.exec("CREATE TABLE IF NOT EXISTS ClientPhone "
			"(phone_id SERIAL primary key, "
			"client_id INT not null references ClientDB(id), "
			"phoneNumber VARCHAR(255))");
		tx01.commit();
	}

	//ћетод, позвол€ющий добавить нового клиента
	//create protection against SQL injection!!!
	void AddNewClient(pqxx::connection& conn) {
		std::string firstName, lastName, email, phoneNum;
		std::cout << "¬ведите им€ и фамилию клиента: ";
		std::cin >> firstName >> lastName;
		std::cout << "¬ведите email: ";
		std::cin >> email;
		std::cout << "¬ведите телефон: ";
		std::cin >> phoneNum;

		pqxx::transaction tx(conn);
		tx.exec("INSERT INTO ""ClientDB(firstName, lastName, email) VALUES ('" + firstName + "','" + lastName + "','"+ email + "')");
		tx.commit();
		pqxx::transaction tx01(conn);
		tx01.exec("INSERT INTO ClientPhone(phoneNumber) VALUES ('"+ phoneNum +"')");
		tx01.commit();
	}
};

int main()
{
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	//create connection

	try {
		pqxx::connection conn("host=localhost "
			"port=5432 "
			"dbname=ClientDB "
			"user=postgres "
			"password=13579");
			std::cout << "Connection performed..." << std::endl;
			std::cout << "Creating table..." << std::endl;
			ClientDB clientDB;
			clientDB.CreateTable(conn);
			std::cout << "Table created" << std::endl;

			std::cout << "Add new client:\n";
			clientDB.AddNewClient(conn);
	}
	catch (std::exception& ex) {
		std::cout << "Connection is not performed. " << ex.what() << std::endl;
	}

	return 0;
}