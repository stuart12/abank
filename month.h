/*	month.h
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
#include <iostream>
using namespace std;
class Account;
class Entry;
class AccountBalance;
class Transactions;
class Month
{
	Month * _prev;
	Month * _next;
	Entry * _entries;
	int n_entries;
	Transactions * const _transactions;
	int const _number;
	AccountBalance * balances; // as at the end of this month
	friend class Transactions;
	friend class Entry;
	Month(Transactions * transactions, int number);
	Month(Month * prev, Month * next = 0);
	void save(ostream & out);
	void move(int, int, int);
	void move_amount(int, int, int);
	int insert(Entry *);
	void remove(Entry *);
	void subtract(Entry const *);
	void add(Entry const *);
	void modify();
	void new_entry(Entry *);
public:
	int num_entries() const { return n_entries; }
	Month * prev();
	Month * next();
	Entry * entries() { return _entries; }
	Entry * new_entry(int day, int month, int year, int f, int t, int a, char const * des);
	int number() const { return _number; }
	int balance(int account) const;
	bool check() const;
};
