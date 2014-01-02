/*	abank.cc
 *	Uses Qt3Support from Qt 4
 *
 *	Copyright 2002, 2004, 2010, 2013, 2014 Stuart Pook (http://www.pook.it/)
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
#include "abank.h"

#include <assert.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <locale.h>

#include <qlineedit.h>
#include <qvalidator.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qlocale.h>
#include <qtextcodec.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QLabel>
#include <Q3BoxLayout>

char * myname;

static struct tm *
now()
{
	time_t now = time(0);
	assert(now != -1);
	return localtime(&now);
}

static int
initial_month()
{
	struct tm * tm = now();
	return tm->tm_mon + (tm->tm_year + 1900) * Transactions::MonthsInAYear;
}

inline QString
format_rounded(int v)
{
	if (v < 0)
		v = (-v + 50) / -100;
	else
		v = (v + 50) / 100;
	return QString().sprintf("%d", v);
}

inline QString
format_amount(int v)
{
	return QString().sprintf("%d.%02d", v / 100, abs(v) % 100);
}

inline QString
format_date(Entry const * e)
{
	return QString().sprintf("%02d/%02d", e->day(), e->month());
}

enum { col_previous = 0, col_current, col_differences, col_number };

class TableBalanceItem : public Q3TableItem
{
	bool _negative;
public:
	TableBalanceItem(Q3Table * t) :
		Q3TableItem(t, Never)
	{
	}
	enum { e_rtti = 35815 };
	virtual int rtti() const { return e_rtti; }
	void paint(QPainter * p, const QColorGroup & cg, const QRect & cr, bool selected )
	{
		if (!_negative)
			Q3TableItem::paint(p, cg, cr, selected);
		else
		{
			QColorGroup ncg = cg;
			ncg.setColor(QColorGroup::Text, Qt::red);
			Q3TableItem::paint(p, ncg, cr, selected);
		}
	}
	void setText(QString const & text)
	{
		if (_negative = (text.at(0) == '-'))
		{
			QString s = text;
			s.remove('-');
			Q3TableItem::setText(s);
		}
		else
			Q3TableItem::setText(text);
	}
};

static void
set_differences(Q3Table * balances, Month * m)
{
	int sum = 0;
	for (int i = 0; i < balances->numRows() - 1; i++)
	{
		Month * p = m->prev();
		int d = p->balance(0) - m->balance(0);
		sum += d;
		balances->setText(i, col_differences, format_rounded(d));
		m = p;
	}
	if (balances->numRows() > 1)
	{
		sum /= balances->numRows() - 1;
		balances->setText(balances->numRows() - 1, col_differences, format_rounded(sum));
	}
}

static void
change_balances(Q3Table * balances, Month *  month, int a1, int a2 = -1)
{
	balances->setText(a1, col_current, format_amount(month->balance(a1)));
	if (a2 >= 0)
		balances->setText(a2, col_current, format_amount(month->balance(a2)));
	if (a1 == 0 || a2 == 0)
		set_differences(balances, month);
}

static void
set_balances(Q3Table * balances, Month * m)
{
	Month const * p = m->prev();
	for (int i = balances->numRows(); i-- > 0; )
	{
		balances->setText(i, col_previous, format_amount(p->balance(i)));
		balances->setText(i, col_current, format_amount(m->balance(i)));
	}
	set_differences(balances, m);
}

static void
set_account(Q3Table * transactions, int r, int c, int current, QStringList const & accounts)
{
#if 1
	Q3TableItem * item = transactions->item(r, c);
	if (item == 0)
	{
		item = new Q3ComboTableItem(transactions, accounts);
		transactions->setItem(r, c, item);
	}
	assert(item->rtti() == 1);
	static_cast<Q3ComboTableItem *>(item)->setCurrentItem(current);
#endif
}

enum { col_date = 0, col_from, col_to, col_amount, col_description, col_count };

struct AmountValidator : public QValidator
{
	static State parse_amount(char const * s, int * v = 0);
	AmountValidator(QObject * parent, char const * name) : QValidator(parent, name)
	{}
	virtual State validate(QString & input, int & pos) const
	{
		return parse_amount(input);
	}
};

AmountValidator::State
AmountValidator::parse_amount(char const * s, int * v)
{
	State state = Intermediate;
	int dummy;
	if (v == 0)
		v = &dummy;
	*v = 0;
	for (; isdigit(*s) || isspace(*s); s++)
	{
		if (isdigit(*s))
		{
			*v = *v * 10 + *s - '0';
			state = Acceptable;
		}
	}
	*v *= 100;
	if (*s == '.' || *s == ',')
	{
		s++;
		if (isdigit(*s))
		{
			state = Acceptable;
			*v += (*s++ - '0') * 10;
			if (isdigit(*s))
				*v += *s++ - '0';
		}
	}
	if (*s)
		return Invalid;
	return state;
}

struct DateValidator : public QValidator
{
	static State parse_date(char const * s, int * const day = 0, int * const month = 0, int * const year = 0);
	DateValidator(QObject * parent) : QValidator(parent, "DateValidator")
	{}
	virtual State validate(QString & input, int & pos) const
	{
		return parse_date(input);
	}
};

DateValidator::State
DateValidator::parse_date(char const * s, int * day, int * month, int * year)
{
	int dummy;
	if (day == 0)
		day = &dummy;
	if (month == 0)
		month = &dummy;
	if (year == 0)
		year = &dummy;
	*day = *month = *year = -1;
	
	State state = Acceptable;
	
	while (isspace(*s))
		s++;
	if (*s == 0)
		return state;
		
	if (!isdigit(*s))
		state = Intermediate;
	else
	{
		for (*day = 0; isdigit(*s); )
			*day = *day * 10 + *s++ - '0';
		if (*day > 31)
			return Invalid;
		if (*day == 0)
			state = Intermediate;
	}
	
	while (isspace(*s))
		s++;
	if (*s == '/')
		for (s++; isspace(*s); s++)
			;
	if (*s == '\0')
		return state;
		
	if (!isdigit(*s))
		state = Intermediate;
	else
	{
		for (*month = 0; isdigit(*s); )
			*month = *month * 10 + *s++ - '0';
		if (*month > 12)
			return Invalid;
		if (*month == 0)
			state = Intermediate;
	}
		
	while (isspace(*s))
		s++;
	if (*s == '/')
		for (s++; isspace(*s); s++)
			;
	if (*s == '\0')
		return state;
		
	if (!isdigit(*s))
		state = Intermediate;
	else
	{
		for (*year = 0; isdigit(*s); )
			*year = *year * 10 + *s++ - '0';
		if (*year > 9999)
			return Invalid;
		if (*year < 1900 && *year > 99)
			state = Intermediate;
	}
	
	while (isspace(*s))
		s++;
	if (*s == '\0')
		return state;
	return Invalid;
}

class TableDateItem : public Q3TableItem
{
      DateValidator validator;
      Entry * _entry;
public:
	TableDateItem(Q3Table * t, const QString & text, Entry * entry) :
		Q3TableItem(t, OnTyping, text),
		validator(table()),
		_entry(entry)
	{
		setReplaceable(false);
	}
	enum { e_rtti = 39846 };
	virtual int rtti() const { return e_rtti; }
	virtual QWidget * createEditor() const
	{
		struct tm * tm = now();
		QLineEdit * le;
		if
		(
			tm->tm_mday == _entry->day()
			&&
			tm->tm_mon + 1 == _entry->month()
			&&
			tm->tm_year + 1900 == _entry->year()
		)
			le = new QLineEdit("", table()->viewport(), "TableAmountItem");
		else
			le = new QLineEdit(text(), table()->viewport(), "TableAmountItem");
		le->setValidator(&validator);
		return le;
	}
	Entry * entry() { return _entry; }
};

class TableAmountItem : public Q3TableItem
{
      AmountValidator validator;
public:
	TableAmountItem(Q3Table * t, const QString & text) :
		Q3TableItem(t, OnTyping, text),
		validator(table(), "TableAmountItem::validator")
	{
		setReplaceable(false);
	}
	enum { e_rtti = 39845 };
	virtual int rtti() const { return e_rtti; }
	virtual QWidget * createEditor() const
	{
		QString s(text());
		int pos = s.find(".00");
		if (pos >= 0)
			s.truncate(pos);
		QLineEdit * le = new QLineEdit(s, table()->viewport());
		le->setValidator(&validator);
		return le;
	}
};

inline void
set_date(Q3Table * transactions, int r, Entry * e)
{
	transactions->setItem(r, col_date, new TableDateItem(transactions, format_date(e), e));
}

inline void
set_amount(Q3Table * transactions, int r, int amount)
{
	transactions->setItem(r, col_amount, new TableAmountItem(transactions, format_amount(amount)));
}

inline void
set_description(Q3Table * transactions, int row, char const * description)
{
	transactions->setText(row, col_description, description);
}

static void
set_entry(Q3Table * transactions, int row, Entry * e, QStringList const & accounts)
{
	set_date(transactions, row, e);
	
	set_account(transactions, row, col_from, e->from(), accounts);
	set_account(transactions, row, col_to, e->to(), accounts);
	
	set_amount(transactions, row, e->amount());
	set_description(transactions, row, e->description());
	transactions->item(row, col_description)->setReplaceable(false);
	
	//transactions->adjustRow(row);
	enum { table_height_fudge = 0 };
	if (table_height_fudge)
		transactions->setRowHeight(row, transactions->rowHeight(row) - table_height_fudge);
}

static void
set_transactions(Q3Table * transactions, Month * month, QStringList const & accounts, int current_account)
{
	transactions->setNumRows(month->num_entries());
	int i = 0;
	for (Entry * e = month->entries(); e; e = e->next())
		if (current_account < 0 || current_account == e->to() || current_account == e->from())
			set_entry(transactions, i++, e, accounts);
	transactions->setNumRows(i);
	transactions->clearSelection();
}

inline void
set_month(QLabel * l, Month const * month)
{
	int m = month->number();
	l->setText(QString().sprintf("%d %d", m % Transactions::MonthsInAYear + 1, m / Transactions::MonthsInAYear));
}

BitmapButton::BitmapButton(QWidget * parent, const char * name, int width, int height, const uchar * bits)
	: QPushButton(parent, name)
{
	enum { xbm_fudge = 6 };
	setPixmap(QBitmap(width, height, bits, true));
	setFixedSize(width + xbm_fudge, height + xbm_fudge);
	setFocusPolicy(Qt::NoFocus);
}

void
Abank::changed_month()
{
//	setUpdatesEnabled(false);
	set_month(&month_label, month);
	set_balances(&balances, month);
	set_transactions(&trans, month, accounts, current_account);
	if (trans.numRows())
	{
		trans.clearSelection();
		trans.ensureCellVisible(0, 0);
	}
//	setUpdatesEnabled(true);
//	repaint();
}

void
Abank::modify()
{
	quit.setPixmap((transactions.modified()
#ifdef CHECK_BALANCES
		&& transactions.check()
#endif
		) ? save_icon : bomb_icon);
	if (!transactions.modified())
		clog << "transactions should be modified!" << endl;
}

static int
fix_year(int year)
{
	if (year >= 0 && year < 100)
	{
		clog << myname << ": year " << year << " changed to ";
		struct tm * tm = now();
		int current_year = tm->tm_year + 1900;
		year += (current_year / 100) * 100;
		if (year - current_year > 50)
			year -= 100;
		clog << year << endl;
	}
	return year;
}

void
Abank::value_changed(int r, int c)
{
	Q3TableItem * item = trans.item(r, c);
	TableDateItem * tdi = static_cast<TableDateItem *>((c == 0) ? item : trans.item(r, 0));
	assert(tdi->rtti() == tdi->e_rtti);
	Entry * e = tdi->entry();
	if (c == col_from || c == col_to)
	{
		assert(item->rtti() == 1);
		Q3ComboTableItem * cti = static_cast<Q3ComboTableItem *>(item);
		int account = cti->currentItem();
		int oaccount;
		if (c == col_from)
		{
			oaccount = e->from();
			if (oaccount != account)
				e->from(account);
		}
		else
		{
			oaccount = e->to();
			if (oaccount != account)
				e->to(account);
		}
		if (oaccount != account)
		{
			change_balances(&balances, e->current_month(), oaccount, account);
			trans.clearSelection();
			modify();
		}
	}
	else
	{
		QString n = trans.text(r, c);
		if (c == col_description)
		{
			n.replace(QChar('\n'), QChar(' '));
			if (n != e->description())
			{
				e->description(n);
				modify();
				set_description(&trans, r, e->description());
			}
		}
		else if (c == col_amount)
		{
			int v;
			if (AmountValidator::parse_amount(n, &v) == AmountValidator::Acceptable && e->amount() != v)
			{
				e->amount(v);
				change_balances(&balances, e->current_month(), e->to(), e->from());
				trans.clearSelection();
				modify();
			}
			set_amount(&trans, r, e->amount());
		}
		else if (c == col_date)
		{
			int day, month, year;
			if (DateValidator::parse_date(n, &day, &month, &year) == DateValidator::Acceptable)
			{
				day = (day > 0) ? day : e->day();
				month = (month > 0) ? month : e->month();
				year = (year > 0) ? year : e->year();
				year = fix_year(year);
				if (day != e->day() || month != e->month() || year != e->year())
				{
					e->date(day, month, year);
					trans.clearSelection();
					modify();
				}
			}
			set_date(&trans, r, e);
		}
		else
			assert(!"bad column");
	}
}

void
Abank::add_entry()
{
	struct tm * tm = now();
	int account = (current_account < 0) ? 0 : current_account;
	int row = trans.numRows();
	Entry * e = month->new_entry(tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, account, account, 0, "");
	trans.insertRows(row);
	set_entry(&trans, row, e, accounts);
	trans.ensureCellVisible(row, 0);
	modify();
}

void
Abank::previous_month()
{
	month = month->prev();
	changed_month();
}

void
Abank::next_month()
{
	month = month->next();
	changed_month();
}

void
Abank::save_or_quit()
{
	if (!transactions.modified())
		exit(0);
	char const * message = transactions.save();
	if (message == 0)
	{
		quit.setPixmap(quit_icon);
		assert(!transactions.modified());
		if (!transactions.check())
			quit.setPixmap(bomb_icon);
	}
	else
	{
		QString s("could not save \"");
		s.append(transactions.file()).append("\": ").append(message);
		QMessageBox error
		(
			"save error",
			s,
			QMessageBox::Warning,
			QMessageBox::Ok | QMessageBox::Default,
			QMessageBox::NoButton,
			QMessageBox::NoButton,
			this,
			"save_error",
			true
		);
		error.exec();
	}
}
	
typedef void (Entry::*Treat)();

static bool
treat_selected(Q3Table * trans, Q3Table * balances, bool remove, Treat func)
{
	if (trans->numSelections() == 0)
		return false;
	Q3TableSelection sel = trans->selection(0);
	Q3Table::SelectionMode const mode = trans->selectionMode();
	trans->setSelectionMode(Q3Table::NoSelection);
	
	Month * m = 0;
	for (int i = sel.bottomRow(); i >= sel.topRow(); i--)
	{
		TableDateItem * di = static_cast<TableDateItem *>(trans->item(i, 0));
		m = di->entry()->current_month();
		(di->entry()->*func)();
		if (remove)
			trans->removeRow(i);
	}
	set_balances(balances, m);
	trans->setSelectionMode(mode);
	return true;
}

void
Abank::move_back_selected()
{
	if (treat_selected(&trans, &balances, true, &Entry::move_back))
		modify();
}

void
Abank::copy_forward_selected()
{
	if (treat_selected(&trans, &balances, false, &Entry::copy_forward))
		modify();
}

void
Abank::move_forward_selected()
{
	if (treat_selected(&trans, &balances, true, &Entry::move_forward))
		modify();
}

void
Abank::delete_selected()
{
	if (treat_selected(&trans, &balances, true, &Entry::remove))
		modify();
}

void
Abank::no_account()
{
	if (balances.numSelections() == 0)
	{
		current_account = -1;
		set_transactions(&trans, month, accounts, current_account);
	}
}

void
Abank::change_account(int section)
{
	if (current_account == section)
	{
		balances.clearSelection();
		current_account = -1;
	}
	else
	{
		Q3TableSelection sel;
		sel.init(section, 0);
		sel.expandTo(section, 1);
		balances.clearSelection();
		balances.addSelection(sel);
		current_account = section;
	}
	set_transactions(&trans, month, accounts, current_account);
}

const
#include "disk_cat.xpm"
const
#include "door2.xpm"
const
#include "bomb.xpm"
#include "left.xbm"
#include "right.xbm"
#include "delete_entry.xbm"
#include "copy_right.xbm"
#include "move_left.xbm"
#include "move_right.xbm"
Abank::Abank(char const * file, bool small) :
	Q3MainWindow(0, "Abank"),
	current_account(-1),
	transactions(file),
	change_month(0, "change_month"),
	prev(this, "prev", left_width, left_height, left_bits),
	next(this, "next", right_width, right_height, right_bits),
	buttons(0, "buttons"),
	month_label("2002", this, "month"),
	create(0, "create"),
	delete_entry(this, "delete_entry", delete_entry_width, delete_entry_height, delete_entry_bits),
	copy_right(this, "copy_right", copy_right_width, copy_right_height, copy_right_bits),
	shift(0, "shift"),
	move_left(this, "move_left", move_left_width, move_left_height, move_left_bits),
	move_right(this, "move_right", move_right_width, move_right_height, move_right_bits),
	quit(this, "quit/save"),
	global(this, 0, 0, "global"),
	top(0, "top"),
	balances(1, col_number, this, "balances"),
	trans(1, col_count, this, "trans"),
	quit_icon(door2),
	save_icon(disk_cat),
	bomb_icon(bomb)
{
	setCaption(QString("abank ").append(file));
	if (!transactions.read())
	{
		new QLabel(QString("could not read transactions from ").append(file), this);
		return;
	}
	if (!transactions.check())
	{
		new QLabel("internal error check after read failed", this);
		return;
	}
	if (transactions.naccounts() == 0)
	{
		new QLabel(QString("no accounts in ").append(file), this);
		return;
	}
	
	month = transactions.month(initial_month());
	
	setFont(QFont("FreeSans", (small) ? 8 : 10));
	change_month.addStretch(1);

	change_month.addWidget(&prev);
	change_month.addStretch(1);
	change_month.addWidget(&next);
	change_month.addStretch(1);
	
	buttons.addStretch(1);
	
	set_month(&month_label, month);
	month_label.setAlignment(Qt::AlignCenter);
	month_label.setFont(QFont(font().family(), font().pointSize() + 2));
	buttons.addWidget(&month_label);
	buttons.addStretch(1);
	
	buttons.addLayout(&change_month);
	buttons.addStretch(1);
	
	create.addStretch(1);
	{
		const
#include "add_entry.xbm"
		BitmapButton * b_add_entry = new BitmapButton(this, "add_entry", add_entry_width, add_entry_height, add_entry_bits);
		connect(b_add_entry, SIGNAL(clicked()), this, SLOT(add_entry()));
		create.addWidget(b_add_entry);
	}
	create.addSpacing(2);
	create.addStretch(1);
	create.addWidget(&delete_entry);
	connect(&delete_entry, SIGNAL(clicked()), this, SLOT(delete_selected()));
	create.addSpacing(2);
	create.addStretch(1);
	create.addWidget(&copy_right);
	connect(&copy_right, SIGNAL(clicked()), this, SLOT(copy_forward_selected()));
	create.addStretch(1);
	
	buttons.addLayout(&create);
	buttons.addStretch(1);
	
	shift.addStretch(1);
	shift.addWidget(&move_left);
	connect(&move_left, SIGNAL(clicked()), this, SLOT(move_back_selected()));
	shift.addStretch(1);
	shift.addWidget(&move_right);
	connect(&move_right, SIGNAL(clicked()), this, SLOT(move_forward_selected()));
	shift.addStretch(1);
	
	buttons.addLayout(&shift);
	buttons.addStretch(1);
	
	quit.setPixmap(quit_icon);
	quit.setFocusPolicy(Qt::NoFocus);
	{
		Q3BoxLayout * lquit = new Q3HBoxLayout(&buttons, 0, "lquit");
		lquit->addStretch(1);
		lquit->addWidget(&quit);
		lquit->addStretch(1);
	}
	buttons.addStretch(1);
	
	top.addStretch(2);
	top.addLayout(&buttons, 2);
	top.addStretch(2);
	
	Q3Table * const b = &balances;
	b->setNumRows(transactions.naccounts());
	b->setTopMargin(0);
	b->setReadOnly(true);
	b->setShowGrid(false);
	b->setFocusPolicy(Qt::NoFocus);
	b->setText(0, col_previous, "-999999.99");
	b->setText(0, col_differences, "9999");
	for (int i = 0; i < transactions.naccounts(); i++)
	{
		QString s = transactions.account(i);
		b->verticalHeader()->setLabel(i, s);
		accounts += s;
		//b->adjustRow(i);
		//b->setRowHeight(i, b->rowHeight(i) - 8);
	}
	b->verticalHeader()->setResizeEnabled(false);
	b->adjustColumn(col_previous);
	b->adjustColumn(col_differences);
	b->setColumnWidth(col_current, b->columnWidth(col_previous));
	b->setSelectionMode(b->NoSelection);
	connect(b->verticalHeader(), SIGNAL(clicked(int)), this, SLOT(change_account(int)));
	connect(b, SIGNAL(selectionChanged()), this, SLOT(no_account()));
	for (int i = 0; i < b->numRows(); i++)
		b->setItem(i, col_differences, new TableBalanceItem(b));
	
	top.addWidget(b);
	top.addStretch(2);

	Q3Table * const t = &trans;
	t->setNumRows(1);
	t->setText(0, col_date, "00/00");
	t->adjustColumn(col_date);
	t->setColumnWidth(col_date, t->columnWidth(col_date) - 4);
	t->setText(0, col_amount, "99999.99");
	t->adjustColumn(col_amount);
	t->setColumnWidth(col_amount, t->columnWidth(col_amount) - 2);
	
#if 1
	t->setText(0, col_from, "Boursorama CB");
#else
	set_account(t, 0, col_from, 0, accounts);
#endif
	t->adjustColumn(col_from);
	t->setColumnWidth(col_to, t->columnWidth(col_from));
	
	t->setColumnStretchable(col_description, true);
//	t->setRowStretchable(0, false);

	t->setLeftMargin(0);
	t->setTopMargin(0);
	t->setSelectionMode(t->MultiRow);
	
	Q3BoxLayout * const g = &global;
	g->addLayout(&top);
	g->addWidget(t, 1);
	
	setMinimumSize((small) ? QSize(236, 279) : QSize(600, 700));
	show();
	t->setNumRows(0);
//	if (small)
//		resize(236, 279);
	
	changed_month();
	
	connect(&prev, SIGNAL(clicked()), this, SLOT(previous_month()));
	connect(&next, SIGNAL(clicked()), this, SLOT(next_month()));
	connect(&quit, SIGNAL(clicked()), this, SLOT(save_or_quit()));
	connect(t, SIGNAL(valueChanged(int, int)), this, SLOT(value_changed(int, int)));
}

int main(int argc, char * argv[])
{
	setlocale(LC_ALL, "");
	// http://lists.trolltech.com/qt-interest/2008-06/thread00094-0.html
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	myname = argv[0];
	QApplication application(argc, argv);
	setlocale(LC_NUMERIC, "C");
	bool small = false;
	if (argv[1] && strcmp(argv[1], "-s") == 0)
	{
		argc--;
		argv++;
		small = true;
	}
	if (argc != 2)
	{
		cerr << "usage: " << myname << " [-s] <accounts>" << endl;
		exit(1);
	}
	Abank abank(argv[1], small);
	
	application.setMainWidget(&abank);

 	return application.exec();
}
