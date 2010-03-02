/*	transactions.cc
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
// $Id: transactions.cc,v 2.11 2009-10-28 08:24:44 stuart Exp $
#include	<stdlib.h>
#include	<unistd.h>
#include	<fstream>
//#include	<strstream>
#include	<string>
#include	<fcntl.h>
#include	<errno.h>
#include	<string.h>
#include	<assert.h>
#include	"transactions.h"
#include	"entry.h"
#include	"month.h"

#if __GNUC__ >= 3 && __GNUC_MINOR__ >= 1
#define NEED_STDIO_FILEBUF 1
#include <ext/stdio_filebuf.h> 
#endif

// http://www.josuttis.com/cppcode/fdstream.hpp.html
// http://www.josuttis.com/cppcode/
// http://www.josuttis.com/cppcode/fdstream.html

char const *
Transactions::save()
{
	int fd;
	if ((fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0666)) < 0)
	{
		if ((fd = open(filename, O_WRONLY | O_CREAT, 0666)) < 0)
			return "could not create";
		
		struct stat info;
		if (fstat(fd, &info) < 0)
		{
			close(fd);
			return "could not fstat";
		}
		if (info.st_ino != filestat.st_ino || info.st_dev != filestat.st_dev)
		{
			close(fd);
			return "file replaced -- refuse to overwrite";
		}
		if (info.st_mtime != filestat.st_mtime || info.st_size != filestat.st_size)
		{
			close(fd);
			return "file modified -- refuse to overwrite";
		}
		if (ftruncate(fd, 0) < 0)
		{
			close(fd);
			return "could not ftruncate";
		}
	}
#ifdef NEED_STDIO_FILEBUF
	__gnu_cxx::stdio_filebuf<char> fb(fd, ios_base::out, BUFSIZ);
	ostream out(&fb);
#elif __GNUC__ == 3 && __GNUC_MINOR__ == 0
	filebuf fb(fdopen(fd, "w"), ios_base::out);
	ostream out(&fb);
#else
	ofstream out(fd);
#endif
	if (!out)
		return "could not open";
	for (int i = 0; i < naccounts(); i++)
		out << "a " << accounts[i] << endl;
	last_month->save(out);
	out.flush();
	if (fstat(fd, &filestat) < 0)
	{
		cerr << "could not fstat " << filename << ": " << strerror(errno) << endl;
		exit(1);
	}
#if defined(NEED_STDIO_FILEBUF) || __GNUC__ == 3 && __GNUC_MINOR__ == 0
	fb.close();
#else
	out.close();
#endif
	(void)close(fd) ; // already closed under linux
	_modified = false;
	return 0;
}

#if 0
Account *
Transactions::make_account_array(Account * p, int n)
{
	Account * a = new Account[n];
	for (int i = 0; p; p = p->next, i++)
		a[i] = *p;
	return a;
}

int Transactions::num_entries(int month_num)
{
	Month const * m = lookup(month_num);
	return (m == 0) ? 0 : m->num_entries();
}

Month const *
Transactions::month(int month_number)
{
	return lookup(month_number);
}

Month *
Transactions::lookup(int month_number)
{
	if (last_month == 0)
	{
		last_month_number = month_number;
		last_month = new Month(num_accounts);
	}
	else
	{
		for (; month_number > last_month_number; last_month_number++)
			last_month = new Month(num_accounts, last_month);
	}
		
	int current_month_number = last_month_number;
	Month * current_month = last_month;
	while (current_month_number > month_number && current_month)
	{
		current_month_number--;
		current_month = current_month->prev(num_accounts);
	}
	return current_month;
}

char const * Transactions::account(int i)
{
	Account * p = accounts;
	while (i-- > 0 && p)
		p = p->next;
	if (p == 0)
		return 0;
	return p->name();
}
int Transactions::naccounts()
{
	int i = 0;
	for (Account * p = accounts; p; p = p->next)
		i++;
	return i;
}
#endif

int
Transactions::read()
{
	ifstream in(filename);
	if (!in)
	{
		cerr << "could not open " << filename << ": " << strerror(errno) << endl;
		return 0;
	}
// HACK, should be fstat
	if (stat(filename, &filestat) < 0)
	{
		cerr << "could not stat " << filename << ": " << strerror(errno) << endl;
		return 0;
	}
	
	char c;
	for (int line = 1; in.get(c); line++)
	{
		char space;
		if (!in.get(space))
		{
			cerr << filename << ":" << line << ": error when wanted a space" << endl;
			return 0;
		}
		if (space != ' ')
		{
			cerr << filename << ":" << line << ": wanted a space, found: ";
			cerr.put(space);
			cerr << " (" << int(space) << ")" << endl;
			return 0;
		}
		switch (c)
		{
		default:
			cerr << filename << ":" << line << ": bad leading character: ";
			cerr.put(c);
			cerr << endl;
			return 0;
		case 'a':
			if (last_month)
			{
				cerr << filename << ":" << line << ": account after entries" << endl;
				return 0;
			}
			accounts.push_back(read_string(in));
			break;
		case 'e':
			int year = read_integer(in);
			int month_number = year * MonthsInAYear + read_integer(in) - 1;
			if (last_month == 0)
				last_month = new Month(this, month_number);
			month(month_number)->new_entry(new Entry(in));
			break;
		}
	}
	_modified = false;
	return 1;
}

Month *
Transactions::month(int n)
{
	while (n > last_month->_number)
		last_month = last_month->next();
	Month * m;
	for (m = last_month; m->_number != n; m = m->prev())
		;
	return m;	
}

#if 0	
Entry const *
Transactions::append(int month_number, int day, int month, int year, int from, int to, int account, char const * description)
{
	Entry * e = new Entry(day, month, year, from, to, account, description);
	append(month_number, e);
	return e;
}

void
Transactions::move_amount(Month * m, int from, int to, int amount)
{
	for (Month * p = last_month; ; p = p->prev(num_accounts))
	{
		p->move(from, to, amount);
		if (p == m)
			break;
	}
}
int
Transactions::set_from(int month_num, int line, int account, int current_account)
{
	_modified = 1;
	Month * m = lookup(month_num);
	Entry * e = m->entry(line);
	int r = e->from();
	e->from(account);
	move_amount(m, r, account, -e->amount());
	return r;
}

int
Transactions::set_to(int month_num, int line, int account)
{
	_modified = 1;
	Month * m = lookup(month_num);
	Entry * e = m->entry(line);
	int r = e->to();
	e->to(account);
	move_amount(m, r, account, e->amount());
	return r;
}


void Transactions::move_back(int month_num, int line)
{
	_modified = 1;
	Month * m = lookup(month_num);
	Entry * e = m->entry(line);
	Month * prev = m->prev(num_accounts);
	m->remove(e);
	prev->append(e);
	prev->add(e);
}

void
Transactions::copy_forward(int month, int line)
{
	Entry const * e = get_entry(month, line);
	int m = e->month();
	int y = e->year();
	if (++m > MonthsInAYear)
	{
		m = 1;
		y++;
	}
	append(month + 1, e->day(), m, y, e->from(), e->to(), e->amount(), e->description());
}

void Transactions::move_forward(int month_num, int line)
{
	_modified = 1;
	Month * next = lookup(month_num + 1);
	Month * m = next->prev(num_accounts);
	Entry * e = m->entry(line);
	m->subtract(e);
	m->remove(e);
	next->append(e);
}

void Transactions::remove(int month_num, int line)
{
	_modified = 1;
	Month * m = lookup(month_num);
	Entry * e = m->entry(line);
	assert(e);
	move_amount(m, e->to(), e->from(), e->amount());
	m->remove(e);
	delete e;
}
#endif

char *
Transactions::read_string(istream & in)
{
#if 0
	ostrstream buffer;
	char c;
	if (!in.get(*buffer.rdbuf()) || !in.get(c) || c != '\n')
		return 0;
	buffer << ends;
	clog << "read_string " << buffer << endl;
	return buffer.str();
#else
	string buffer;
	getline(in, buffer, '\n');
	char * p = new char[buffer.length() + 1];
	buffer.copy(p, string::npos);
	p[buffer.length()] = 0;
	return p;
#endif
		
}
int
Transactions::read_integer(istream & in)
{
	unsigned v;
	char c;
	if (!(in >> v) || !in.get(c) || c != ' ')
		return -1;
	return v;
}

/*
Entry const *
Transactions::set_date(int month_num, int line, int day, int month, int year)
{
	Entry * e = lookup(month_num)->entry(line);
	if (day > 0)
		e->day(day);
	if (month > 0)
		e->month(month);
	if (year > 0)
		e->year(year);
	_modified = 1;
	return e;
}
*/

bool
Transactions::check() const
{
	bool ok = true;
	if (last_month && last_month->_next != 0)
	{
		clog << "the last_month has a following month" << endl;
		ok = false;
	}
	for (Month const * m = last_month; m; m = m->_prev)
		ok = m->check() && ok;
	return ok;
}

void
Transactions::modify()
{
	_modified = true;
}
