#	Makefile.am, Copyright (c) 2010, 2011, 2014 Stuart Pook (http://www.pook.it/)
#
#	This file is part of ABank.
#	
#	ABank is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#	
#	ABank is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#	
#	You should have received a copy of the GNU General Public License
#	along with ABank.  If not, see <http://www.gnu.org/licenses/>.

bin_PROGRAMS = abank
abank_SOURCES = abank.cc entry.cc month.cc transactions.cc
abank_LDADD = $(LIBOBJS) $(QT3SUPPORT_LIBS)
abank_CXXFLAGS = $(QT3SUPPORT_CFLAGS)
BUILT_SOURCES = abank.h

MOC = moc
abank.h: abank.moc
	$(MOC) -f $< -o $@
abank.o: abank.h
