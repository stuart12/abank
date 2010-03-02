/*	month.cc
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
#include <assert.h>
#include "month.h"
#include "entry.h"
#include "accountbalance.h"
#include "transactions.h"
void
Month::save(ostream & out)
{
	if (_prev)
		_prev->save(out);
	for (Entry * p = _entries; p; p = p->_next)
	{
		out << "e " << _number / Transactions::MonthsInAYear << " ";		
		out << _number % Transactions::MonthsInAYear + 1 << " ";
		p->save(out);
		out.put('\n');
	}
}

int
Month::insert(Entry * e)
{
	int i = 0;
	Entry ** p;
	for (p = &_entries; *p && (*p)->difference(e) <= 0; p = &(*p)->_next)
		i++;
	if ((e->_next = *p))
		e->_next->_prev = &e->_next;
	e->_prev = p;
	*p = e;
	n_entries++;
	e->_current_month = this;
	return i;
}

Month *
Month::next()
{
	if (_next == 0)
		_transactions->last_month = _next = new Month(this);
	return _next;
}

Month *
Month::prev()
{
	if (_prev == 0)
		_prev = new Month(0, this);
	return _prev;
}

Month::Month(Transactions * transactions, int number) :
	_prev(0),
	_next(0),
	_entries(0),
	n_entries(0),
	_transactions(transactions),
	_number(number)
{
	balances = new AccountBalance[transactions->naccounts()];
}

Month::Month(Month * prev, Month * next) :
	_prev(prev),
	_next(next),
	_entries(0),
	n_entries(0),
	_transactions((prev) ? prev->_transactions : next->_transactions),
	_number((prev) ? prev->_number + 1: next->_number - 1)
{
	balances = new AccountBalance[_transactions->naccounts()];
	if (prev)
		for (int i = _transactions->naccounts(); i-- > 0; )
			balances[i].add(prev->balances[i].balance());
	if (next)
		assert(next->_number - 1 == _number && _transactions == next->_transactions);
}

/*Entry const *
Month::entry(int i, int account) const
{
	Entry const * p = entries->account(account);
	while (p && i-- > 0)
		p = p->next(account);
	return p;
}

Entry *
Month::entry(int i)
{
	Entry * p = entries->account(account);
	while (p && i-- > 0)
		p = p->next(account);
	return p;
}*/

void Month::remove(Entry * e)
{
	n_entries--;
	*e->_prev = e->_next;
	if (e->_next)
		e->_next->_prev = e->_prev;
}

void
Month::move(int from, int to, int amount)
{
	balances[from].add(-amount);
	balances[to].add(amount);
}

void
Month::move_amount(int from, int to, int amount)
{
	for (Month * m = this; m; m = m->_next)
		m->move(from, to, amount);
}

void
Month::add(Entry const * e)
{
	move(e->from(), e->to(), e->amount());
}

void
Month::subtract(Entry const * e)
{
	move(e->from(), e->to(), -e->amount());
}

int
Month::balance(int i) const
{
	return balances[i].balance();
}

void
Month::modify()
{
	_transactions->modify();
}

void
Month::new_entry(Entry * e)
{
	move_amount(e->from(), e->to(), e->amount());
	insert(e);
	modify();
}

Entry *
Month::new_entry(int day, int month, int year, int f, int t, int a, char const * des)
{
	Entry * e = new Entry(day, month, year, f, t, a, des);
	new_entry(e);
	return e;
}

static ostream &
wmonth(ostream & o, int month)
{
	return o << "month " << month % 12 + 1 << ' ' << month / 12;
}

bool
Month::check() const
{
	bool ok = true;
	int n = _transactions->naccounts();
	int v[_transactions->naccounts()];
	for (int i = 0; i < n; i++)
		v[i] = (_prev) ? int(_prev->balances[i]) : 0;
	int count = 0;
	Entry * const * p = &_entries;
	for
	(
		Entry * e = _entries;
		e;
		p = &e->_next, e = e->_next
	)
	{
		v[e->_from] -= e->_amount;
		v[e->_to] += e->_amount;
		if (e->_prev != p)
		{
			wmonth(clog, _number) << " bad back link in entry " << count << endl;
			ok = false;
		}
		count++;
	}
	if (count != num_entries())
	{
		clog << "month " << _number % 12 + 1 << ' ' << _number / 12 << " should have ";
		clog << num_entries() << " entries but has " << count << endl;
		ok = false;
	}
	
	for (int i = 0; i < n; i++)
		if (v[i] != balances[i])
		{
			wmonth(clog, _number) << " account ";
			clog << _transactions->account(i) << " should be " << v[i];
			clog << " but is " << balances[i] << endl;
			ok = false;
		}
	if (_prev)
	{
		if (_prev->_next != this)
		{
			wmonth(clog, _number) << " _prev->_next != this" << endl;
			ok = false;
		}
		if (_prev->_number + 1 != _number)
		{
			wmonth(clog, _number) << " preceding month has number ";
			wmonth(clog, _prev->_number) << endl;
			ok = false;
		}
	}
	if (_next)
	{
		if (_next->_prev != this)
		{
			wmonth(clog, _number) << " _next->_prev != this" << endl;
			ok = false;
		}
		if (_next->_number - 1 != _number)
		{
			wmonth(clog, _number) << " following month has number ";
			wmonth(clog, _next->_number) << endl;
			ok = false;
		}
	}
	return ok;
}
