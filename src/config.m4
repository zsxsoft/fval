dnl $Id$

PHP_ARG_ENABLE(fval, whether to enable fval support,
[  --enable-fval           Enable fval support])

if test "$PHP_fval" != "no"; then
  PHP_NEW_EXTENSION(fval, fval.c, $ext_shared)
fi
