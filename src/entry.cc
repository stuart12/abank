/*	entry.cc
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
#include	<assert.h>
#include	<iostream>
#include	"entry.h"
#include	"month.h"
#include	"transactions.h"

int
Entry::difference(Entry const * e) const
{
	int i;
	if ((i = year() - e->year()))
		return i;
	if ((i = month() - e->month()))
		return i;
	if ((i = day() - e->day()))
		return i;
	return strcmp(_description, e->description());
}

Entry::Entry(istream & in) :
	_next(0),
	_prev(0),
	_current_month(0)
{
	_year = Transactions::read_integer(in);
	_month = Transactions::read_integer(in);
	_day = Transactions::read_integer(in);
	_from = Transactions::read_integer(in);
	_to = Transactions::read_integer(in);
	_amount = Transactions::read_integer(in);
	_description = Transactions::read_string(in);
}

void Entry::save(ostream & out) const
{
	out << _year << " " << _month << " " << _day << " ";
	out <<_from << " " << _to << " " << _amount << " " << _description;
}

Entry::Entry(int day, int month, int year, int from, int to, int amount, char const * des) :
	_day(day),
	_month(month),
	_year(year),
	_amount(amount),
	_from(from),
	_to(to),
	_description(strcpy(new char[strlen(des) + 1], des)),
	_next(0),
	_prev(0),
	_current_month(0)
{
	assert(_day >= 1 && _day <= 31);
	assert(_month >= 1 && _month <= 12);
	assert(_year >= 1900 && _year <= 9999);
}

void
Entry::to(int account)
{
	_current_month->move_amount(_to, account, _amount);
	_to = account;
	modify();
}

void
Entry::from(int account)
{
	_current_month->move_amount(_from, account, -_amount);
	_from = account;
	modify();
}

void
Entry::amount(int amount)
{
	_current_month->move_amount(_from, _to, amount - _amount);
	_amount = amount;
	modify();
}

void
Entry::move_forward()
{
	Month * m = _current_month;
	m->remove(this);
	m->next()->insert(this);
	m->subtract(this); // must subtract after the insert
	modify();
}

void
Entry::move_back()
{
	Month * m = _current_month;
	m->remove(this);
	m->prev()->add(this);
	m->prev()->insert(this);
	modify();
}

void
Entry::copy_forward()
{
	int m = _month;
	int y = _year;
	if (++m > 12)
	{
		m = 1;
		y++;
	}
	_current_month->next()->new_entry(_day, m, y, _from, _to, _amount, _description);
}

void
Entry::remove()
{
	_current_month->move_amount(_to, _from, _amount);
	_current_month->remove(this);
	modify();
	_current_month = 0;
	_next = 0;
	_prev = 0;
	_to = _from = _amount = 0;
	delete this;
}

void
Entry::date(int day, int month, int year)
{
	if (day != _day || month != _month || year != _year)
	{
		_day = day;
		_month = month;
		_year = year;
		_current_month->remove(this);
		_current_month->insert(this);
	}
	modify();
}

void
Entry::modify()
{
	_current_month->modify();
}

void
Entry::description(char const * s)
{
	delete [] _description;
	_description = strcpy(new char[(strlen(s) + 1)], s);
	modify();
}
