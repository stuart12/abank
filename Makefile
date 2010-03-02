#	Makefile
#
#	Copyright 2002, 2004, 2010 Stuart Pook
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

CXXFLAGS += -DCHECK_BALANCES $O
os := ${shell uname}
installed = ${HOME}/lib/abank
CPPFLAGS = -I${qt}/include
moc = ${qt}/bin/moc
ifeq (${cross},arm-linux)
	CXX=arm-linux-g++
	inc := ${patsubst %/bin/${CXX}, %-sysroot/usr/include/qt, ${shell set -- `type ${CXX}`; echo $$3}}
	CPPFLAGS=-I${inc}
	LDLIBS= -lqt
	qt=/usr/local/packages/qt
else
ifeq (${os},Linux)
        arch := ${shell uname -m}
ifeq (${arch},armv4l)
                qt=/opt/devel/qt-3.0.5
		libs=/usr/lib/lib
		LDLIBS = -lqt -lICE
		args = -s
else
		qt=/usr
		CPPFLAGS = -I/usr/include/qt3
		CXXFLAGS += -O
		libs=${qt}/lib
		LDLIBS = -lqt
		moc = moc
endif
	LDFLAGS = -L/usr/X11R6/lib -L${libs} -Xlinker -R/usr/X11R6/lib -Xlinker -R${libs} -g
	CXXFLAGS += -g -Wall
	env=env MALLOC_CHECK_=1
else
ifeq (${os},SunOS)
	qt=/usr/local/packages/qt
	LDFLAGS = -R${qt}/lib -L${qt}/lib #-L/usr/local/lib -R/usr/local/lib
	LDLIBS = -lqt -lGL
	CXX = /usr/local/packages/gcc/bin/g++
else
ifeq (${os},NetBSD)
	qt=/usr/X11R6/qt3
	CXXFLAGS += -O
	LDFLAGS = -Xlinker -R${qt}/lib -L${qt}/lib -L/usr/pkg/lib -L/usr/X11R6/lib
	LDLIBS = -lqt-mt
	env=env MALLOC_OPTIONS=A
else
ifeq (${os},Darwin)
	qt=/sw/
	CPPFLAGS=-I/sw/include/qt
	LDLIBS = -lqt-mt
	LDFLAGS = -L/sw/lib
else
endif
endif
endif
endif
endif

name = a.out
objs = abank.o transactions.o month.o entry.o

run: ${name} test
	${env} ./${name} ${args} test
	
test: accounts
	cp accounts $@
	
%.h: %.moc
	${moc} -f -o $@ $<

all: $(name)

$(name) : $(objs)
	@${RM} -f core
	$(CXX) $(LDFLAGS) -o $@ $(objs) ${LDLIBS}

$(objs) : transactions.h month.h entry.h accountbalance.h

abank.o : add_entry.xbm copy_right.xbm delete_entry.xbm \
	left.xbm move_left.xbm move_right.xbm right.xbm bomb.xpm
abank.o: abank.h

clean:
	$(RM) $(name) $(objs) core abank.h
	
${installed} : ${name}
	cp ${name} $@
	
install: ${installed}
	
.PHONY: all clean install run

.DELETE_ON_ERROR:
