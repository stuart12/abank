/*	entry.h
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
#include	<iostream>
#include	<string.h>
#ifndef ABANK_entry_h
#define ABANK_entry_h ABANK_entry_h
using namespace std;
class Month;
class Entry
{
	short _day;
	short _month;
	short _year;
	int _amount;
	short _from;
	short _to;
	char * _description;
	Entry * _next;
	Entry * * _prev;
	friend class Month;
	friend class Transactions;
	Month * _current_month;
	Entry(int day, int month, int year, int f, int t, int a, char const * des);
	~Entry() { delete [] _description; }
	Entry(istream & in);
	void save(ostream & out) const;
	void modify();
public:
	Entry const * next() const { return _next; }
	Entry * next() { return _next; }
	Month * current_month() { return _current_month; }
	void date(int day, int month, int year);
	short day() const { return _day; }
	short month() const { return _month; }
	short year() const { return _year; }
	int amount() const { return _amount; }
	void amount(int);
	void from(int);
	int from() const { return _from; }
	int to() const { return _to; }
	void to(int);
	char const * description() const { return _description; }
	void description(char const * s);
	int difference(Entry const * e) const;
	void move_forward();
	void move_back();
	void copy_forward();
	void remove();
};
#endif
