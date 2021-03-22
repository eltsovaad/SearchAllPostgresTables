#include <libpq-fe.h>
#include <string>
#include<iostream>
using namespace std;

#define UNUSED(x) (void)(x)


static PGconn* conn = NULL;
static PGresult* res = NULL;

static void terminate(int code, PGresult* result)
{
	if (code != 0)
		fprintf(stderr, "%s\n", PQerrorMessage(conn));

	if (result != NULL)
		PQclear(result);
	system("pause");
	exit(code);
}

static void terminate(int code, PGconn* connection)
{
	if (code != 0)
		fprintf(stderr, "%s\n", PQerrorMessage(conn));

	if (connection != NULL)
		PQfinish(connection);
	system("pause");
	exit(code);
}

static void clearRes(PGresult* result)
{
	PQclear(result);
	result = NULL;
}

static void
processNotice(void *arg, const char *message)
{
	UNUSED(arg);
	UNUSED(message);

	// do nothing
}

int main()
{
	conn = PQconnectdb("user=postgres password=aaa host=192.168.40.216 dbname=zabbix");
	if (PQstatus(conn) != CONNECTION_OK)
		terminate(1, conn);
	else
		cout << "Connected!" << endl;

	PQsetNoticeProcessor(conn, processNotice, NULL);

	res = PQexec(conn, "select table_name, column_name from information_schema.columns where table_schema = 'public'");
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		terminate(1,res);

	int nrows = PQntuples(res);
	cout << "Found " << nrows << " columns to check" << endl;
	cout << "Value to find: ";
	string value;
	cin >> value;
	while (cin.fail()) {
		cin.clear();
		cin.ignore(cin.rdbuf()->in_avail());
		cout << "Try again: ";
		cin >> value;
	}
	cout << "Search started!" << endl;
	for (int i = 0; i < nrows; i++)
	{
		string table_name = PQgetvalue(res, i, 0);
		string column_name = PQgetvalue(res, i, 1);
		string stringReq = "select "+column_name+" from " + table_name + " where "+column_name+" = "+value;
	    const char *charReq = stringReq.c_str();
		PGresult* resTemp = NULL;
		resTemp = PQexec(conn, charReq);
		if (PQresultStatus(resTemp) != PGRES_TUPLES_OK) {
			clearRes(resTemp);
		}
		else {
			int numberOfRows = PQntuples(resTemp);
			if (numberOfRows > 0) {
				cout << "	Table: " << table_name << endl;
				cout << "	Column: " << column_name << endl;
				cout << "	"<<numberOfRows << " rows" << endl;
			}
			clearRes(resTemp);
		}
		if (i % 100 == 0) {
			cout << i << " columns checked" << endl;
		}
		
	}

	cout << "The search ended!" << endl;

	clearRes(res);
	terminate(0,conn);
	system("pause");
	return 0;
}