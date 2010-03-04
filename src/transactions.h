/*	transactions.h
 *
 *	Copyright 2002, 2004, 2010 Stuart Pook
 *
 *	This file is part of ABank.
 *
 *	ABank is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *	
 *	ABank is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *	
 *	You should have received a copy of the GNU General Public License
 *	along with ABank.  If not, see <http://www.gnu.org/licenses/>.
 */
#include	<vector>
#include	<iostream>
#include	<string>
#include	<sys/types.h>
#include	<sys/stat.h>
using namespace std;
class Month;
class Entry;
#ifndef Transactions_h
#define Transactions_h
class Transactions
{
	vector<char const *> accounts;
	Month * last_month;
	char * filename;
	struct stat filestat;
	bool _modified;
	void modify();
	Month * lookup(int month);
	void move_amount(Month * m, int from, int to, int amount);
	void append(int month, Entry * e);
	int num_entries(int month);
	Entry const * get_entry(int month, int line, int account = -1);
	friend class Month;
public:
	int naccounts() const { return accounts.size(); }
	char const * account(int i) const { return accounts[i]; }
	enum { MonthsInAYear = 12 };
	char const * save();
	Month * month(int month);
	Transactions(char const * fname) :
		last_month(0),
		filename(strcpy(new char[strlen(fname) + 1], fname))
	{
	}
	char const * file() const { return filename; }
	int modified() const { return _modified; }
	static char * read_string(istream & in);
	static int read_integer(istream & in);
	int read();
	bool check() const;
//	void remove(int month, int entry);
//	void move_forward(int month, int entry);
//	void copy_forward(int month, int line);
//	void move_back(int month, int entry);
//	Entry const * append(int month_number, int day, int month, int year, int from, int to, int account, char const * description);
//	void set_description(int month, int line, char const *);
//	Entry const * set_amount(int month, int line, int amount);
//	int set_from(int month, int line, int from); // returns the old account
//	int set_to(int month, int line, int to); // returns the old account
//	Entry const * set_date(int month, int line, int day, int month, int year);
};
#endif
