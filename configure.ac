dnl	configure.ac, Copyright (c) 2010 Stuart Pook
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
AC_INIT([abank], [1.0], [stuart.pook@example.net])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
Makefile
src/Makefile
])
PKG_CHECK_MODULES([PACKAGES], [qt-mt >= 3.3.0])
AC_OUTPUT
