dnl	configure.ac, Copyright (c) 2014 Stuart Pook (http://www.pook.it/)
dnl
dnl	This file is part of ABank.
dnl	
dnl	ABank is free software: you can redistribute it and/or modify
dnl	it under the terms of the GNU General Public License as published by
dnl	the Free Software Foundation, either version 3 of the License, or
dnl	(at your option) any later version.
dnl	
dnl	ABank is distributed in the hope that it will be useful,
dnl	but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl	GNU General Public License for more details.
dnl	
dnl	You should have received a copy of the GNU General Public License
dnl	along with ABank.  If not, see <http://www.gnu.org/licenses/>.
AC_INIT([abank], [2.0], [stuart.pook@example.net])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
Makefile
src/Makefile
])
dnl http://qt-project.org/doc/qt-4.8/porting4.html
PKG_CHECK_MODULES([QT3SUPPORT], Qt3Support)
AC_OUTPUT
