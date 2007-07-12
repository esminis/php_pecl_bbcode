dnl
dnl $Id$
dnl

PHP_ARG_ENABLE(bbcode,whether to enable C based BBCode support,
[  --enable-bbcode         Enable BBCode support])

if test "$PHP_BBCODE" = "yes"; then
  AC_DEFINE(HAVE_BBCODE,1,[Whether you want BBCode support])
  PHP_NEW_EXTENSION(bbcode, php_bbcode.c bstrlib.c bbcode.c, $ext_shared)
fi
