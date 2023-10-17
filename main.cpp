#include <iostream>
#include <pqxx/pqxx>
#include <Windows.h>
#pragma execution_character_set("utf-8")

class ClientDB 
{
public:
	
	//commitchanges
	void Commit(pqxx::connection& conn, std::string query) {
		pqxx::transaction tx(conn);
		tx.exec(query);
		tx.commit();
	}

	//print data
	void PrintData(pqxx::connection& conn, std::string& query, std::string& id, std::string& firstName, std::string& lastName, std::string& email) {
		pqxx::transaction tx(conn);
		auto result = tx.query<std::string, std::string, std::string, std::string>(query);
		for (auto& row : result) {
			id = std::get<0>(row);
			firstName = std::get<1>(row);
			lastName = std::get<2>(row);
			email = std::get<3>(row);
			std::cout << "������ �������:\n"
				<< id << '\t' << firstName << '\t' << lastName << '\t' << email << std::endl;
		}
		tx.commit();
	}

	//�����, ��������� ��������� �� (�������)
	void CreateTable(pqxx::connection& conn) 
	{
		pqxx::transaction tx(conn);
		tx.exec("CREATE TABLE IF NOT EXISTS ClientList "
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
			"client_id INT not null references ClientList(id), "
			"phoneNumber VARCHAR(255))");
		tx01.commit();
	}

	//�����, ����������� �������� ������ �������
	//create protection against SQL injection!!!
	void AddNewClient(pqxx::connection& conn) {
		std::string firstName, lastName, email, phoneNum;
		std::cout << "������� ��� � ������� �������: ";
		std::cin >> firstName >> lastName;
		std::cout << "������� email: ";
		std::cin >> email;
		std::cout << "������� �������: ";
		std::cin >> phoneNum;

		pqxx::transaction tx(conn);

		std::string query01 = "INSERT INTO ""ClientList(firstName, lastName, email) VALUES ('" + tx.esc(firstName) + "','" + tx.esc(lastName) + "','" + tx.esc(email) + "') returning id";//
		auto result = tx.exec(query01);
		tx.commit();
		pqxx::transaction tx01(conn);
		//client_id should be taken from ClientDB
		//tx01.esc(x) �� ���������, ��  ��� �����, �� ����� ������� � ���� ������
		std::tuple<std::string> x = result[0].as<std::string>();
		tx01.exec("INSERT INTO ClientPhone(client_id, phoneNumber) VALUES ('"+tx01.esc(std::get<0>(x)) + "','" + tx01.esc(phoneNum) + "')");
		tx01.commit();
		std::cout << "������ ���������" << std::endl;
	}


	//find same phone
	bool ComparePhone(pqxx::connection& conn, std::string& phoneNum, std::string& id) {
		pqxx::transaction tx(conn);
		auto compare = tx.query<std::string>("Select phonenumber from clientphone WHERE client_id = " + id + "");
		for (auto& row : compare) {
			std::string result = std::get<0>(row);
			if (phoneNum == result) {
				std::cout << "����� ��� ����������!" << std::endl;
				return true;
			}
		}
		return false;
	}
	//�����, ����������� �������� ������� ��� ������������� �������
	void AddPhone(pqxx::connection& conn, std::string& id)
	{
		//std::string query = "SELECT id from ClientList WHERE id = "+id+"";
		//tx.commit();
		std::cout << "������� ����� �������� ��� ������� � id " << id <<":" << std::endl;
		std::string phoneNum = "";
		std::cin >> phoneNum;
		if (ComparePhone(conn, phoneNum, id)) 
		{
		}
		else {
				pqxx::transaction tx(conn);
				//tx01.esc(x) �� ���������, ��  ��� �����, �� ����� ������� � ���� ������
				tx.exec("INSERT INTO ClientPhone(client_id, phoneNumber) VALUES ('" + tx.esc(id) + "','" + tx.esc(phoneNum) + "')");
				tx.commit();
				std::cout << "������� ��������" << std::endl;
			}
	}

	//�����, ����������� �������� ������ � �������
	void ChangeClientData(pqxx::connection& conn, std::string& id)
	{
		std::string firstname = "", lastname = "", email = "", phoneNum = "", query="", compare="";
		char field=' ';
		int num = 1;
		
		do
		{
			pqxx::transaction tx(conn);
			std::cout << "�������� ����, ������� ������ ��������:\n1.��� 2.������� 3.Email 4.�������" << std::endl;
			std::cin >> field;
			switch (field) {
			case '1': std::cout << "������� ����� ���: ";
				std::cin >> firstname;
				query = "UPDATE ClientList SET firstName = '" + tx.esc(firstname) + "' WHERE id = " + tx.esc(id) + "";
				tx.exec(query);
				tx.commit();
				//Commit(conn, query);
				break;
			case '2': std::cout << "������� ����� �������: ";
				std::cin >> lastname;
				query = "UPDATE ClientList SET lastName = '" + tx.esc(lastname) + "' WHERE id = " + tx.esc(id) + "";
				tx.exec(query);
				tx.commit();
				//Commit(conn, query);
				break;
			case '3': std::cout << "������� ����� email: ";
				std::cin >> email;
				query = "UPDATE ClientList SET email = '" + tx.esc(email) + "' WHERE id = " + tx.esc(id) + "";
				tx.exec(query);
				tx.commit();
				//Commit(conn, query);
				break;
			case '4': std::cout << "������� ����� �������: ";
				std::cin >> phoneNum;
				tx.commit();
				if (ComparePhone(conn, phoneNum, id)) {
					std::cout << "������ ����� �������� ��� ���� � ����" << std::endl;
					break;
				}
				else {
						pqxx::transaction tx01(conn);
						query = "UPDATE ClientPhone SET phoneNumber = '" + tx01.esc(phoneNum) + "' WHERE client_id = " + tx01.esc(id) + "";
						tx01.exec(query);
						tx01.commit();
					}
				//Commit(conn, query);
				break;
			default: std::cout << "������� � ����� ������� �� ����������" << std::endl;
				break;
			}
			
			std::cout << "������� 1, ���� ������ ���������� ��� 0, ����� �����." << std::endl;
			std::cin >> num;
			std::cout << num << std::endl;
		} while (num == 1); //�� ����������� num
			std::cout << "������ ��������" << std::endl;
			
	}
	
	//�����, ����������� ������� ������� ��� ������������� �������
	void DeletePhone(pqxx::connection& conn, std::string& id) 
	{
		std::string query = "DELETE from ClientPhone WHERE client_id = " + id + "";
		Commit(conn, query);
		std::cout << "����� �������� ��� ������� � id " << id << " ��� ������" << std::endl;
	}

	//�����, ����������� ������� ������������� �������
	void DeleteClient(pqxx::connection& conn, std::string& id) {
		DeletePhone(conn, id);
		std::string query = "DELETE from ClientList WHERE id = " + id + "";
		Commit(conn, query);
		std::cout << "������ � id " << id << " ��� ������" << std::endl;
	}

	//�����, ����������� ����� ������� �� ��� ������ (�����, �������, email-� ��� ��������)
	void FindData(pqxx::connection& conn)
	{
		int num;
		std::string id="", query = "", firstName = "", lastName = "", email = "", phoneNum = "";
		std::cout << "������� ������ �������, �������� ������ �����: 1. �� �����\n2.�� �������\n3.�� email\n4.�� ��������\n";
		std::cin >> num;
		switch (num) {
		case 1: std::cout << "������� ��� �������, �������� ������ �����: ";
			std::cin >> firstName;
			query = "SELECT id, firstName, lastName, email from ClientList WHERE firstName LIKE '" + firstName + "'";
			PrintData(conn, query, id, firstName, lastName, email);
			break;
		case 2: std::cout << "������� ������� �������, �������� ������ �����: ";
			std::cin >> lastName;
			query = "SELECT id, firstName, lastName, email from ClientList WHERE firstName LIKE '" + lastName + "'";
			PrintData(conn, query, id, firstName, lastName, email);
			break;
		case 3: std::cout << "������� email �������, �������� ������ �����: ";
			std::cin >> email;
			query = "SELECT id, firstName, lastName, email from ClientList WHERE firstName LIKE '" + email + "'";
			PrintData(conn, query, id, firstName, lastName, email);
			break;
		case 4: std::cout << "������� ������� �������, �������� ������ �����: ";
			std::cin >> phoneNum;
			query = "SELECT ph.phonenumber, c.firstName, c.lastName, c.email "
				"from ClientList c "
				"join clientphone ph ON  ph.client_id = c.id "
				"WHERE ph.phonenumber LIKE '"+ phoneNum +"' group by ph.phonenumber, c.firstName, c.lastName, c.email";
			PrintData(conn, query, id, firstName, lastName, email);
			break;
		default: std::cout << "������ ������� �� ����������" << std::endl;
			break;
		}
		
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
			try {
				clientDB.CreateTable(conn);
				std::cout << "������� �������" << std::endl;
			}
			catch (std::exception& ex) {
				std::cout << "������� ��� ����������" << ex.what() << std::endl;
			}
			std::string id;
			
			std::cout << "�������� ������ �������:\n";
			clientDB.AddNewClient(conn);
			std::cout << "������� id ������� ��� ���������� ��������: ";
			std::cin >> id;
			clientDB.AddPhone(conn, id);
			std::cout << "������� id ������� ��� ��������� ������: ";
			std::cin >> id;
			clientDB.ChangeClientData(conn, id);
			std::cout << "������� id ������� ��� �������� ��������: ";
			std::cin >> id;
			clientDB.DeletePhone(conn, id);
			std::cout << "������� id ������� ��� ��� ��������: ";
			std::cin >> id;
			clientDB.DeleteClient(conn, id);
			std::cout << "����� �������:\n";
			clientDB.FindData(conn);

	}
	catch (std::exception& ex) {
		std::cout << "Connection is not performed. " << ex.what() << std::endl;
	}

	return 0;
}