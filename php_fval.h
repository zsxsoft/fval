/*
   +----------------------------------------------------------------------+
   | Fval                                                                 |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017 zsx                                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author:  zsx    <zsx@zsxsoft.com>                                    |
   +----------------------------------------------------------------------+
*/

#ifndef PHP_FVAL_H
#define PHP_FVAL_H

extern zend_module_entry fval_module_entry;
#define phpext_fval_ptr &fval_module_entry

#ifdef PHP_WIN32
#define PHP_FVAL_API __declspec(dllexport)
#else
#define PHP_FVAL_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define PHP_FVAL_VERSION "0.0.1"

/* it's important that make sure
 * this value is not used by Zend or
 * any other extension agianst string */
#define IS_STR_FVAL_POSSIBLE    (1<<7)

#if PHP_VERSION_ID > 70000
# if PHP_VERSION_ID < 70100
# define PHP_7_0  1
# define PHP_7_1  0
# elif PHP_VERSION_ID < 70200
# define PHP_7_0  0
# define PHP_7_1  1
# elif PHP_VERSION_ID < 70300
# undef IS_STR_FVAL_POSSIBLE
/* Coflicts with GC_COLLECTABLE which is introduced in 7.2 */
# define IS_STR_FVAL_POSSIBLE (1<<6)
# define PHP_7_0  0
# define PHP_7_1  2
# else
# error "Unsupported PHP Version ID:" PHP_VERSION_ID
# endif
#else
# error "Unsupported PHP Version ID:" PHP_VERSION_ID
#endif

#define FVAL_MARK(str)		(GC_FLAGS((str)) |= IS_STR_FVAL_POSSIBLE)
#define FVAL_POSSIBLE(str) (GC_FLAGS((str)) & IS_STR_FVAL_POSSIBLE)
#define FVAL_CLEAN(str)  	(GC_FLAGS((str)) &= ~IS_STR_FVAL_POSSIBLE)

#define FVAL_OP1_TYPE(opline)	(opline->op1_type)
#define FVAL_OP2_TYPE(opline)	(opline->op2_type)


#if PHP_7_0
#define FVAL_RET_USED(opline) (!((opline)->result_type & EXT_TYPE_UNUSED))
#define FVAL_ISERR(var)       (var == &EG(error_zval))
#define FVAL_ERR_ZVAL(var)    (var = &EG(error_zval))
#elif PHP_7_1
#define FVAL_RET_USED(opline) ((opline)->result_type != IS_UNUSED)
#define FVAL_ISERR(var)       (Z_ISERROR_P(var))
#define FVAL_ERR_ZVAL(var)    (ZVAL_ERROR(var))
#endif

typedef zval* fval_free_op;

PHP_MINIT_FUNCTION(fval);
PHP_MSHUTDOWN_FUNCTION(fval);
PHP_RINIT_FUNCTION(fval);
PHP_RSHUTDOWN_FUNCTION(fval);
PHP_MINFO_FUNCTION(fval);

typedef void (*php_func)(INTERNAL_FUNCTION_PARAMETERS);

ZEND_BEGIN_MODULE_GLOBALS(fval)
  zend_bool enable;
  zend_bool disable_eval;
  zend_bool disable_functions;
  int       error_level;
  HashTable fval_wrote_files;
ZEND_END_MODULE_GLOBALS(fval)

#ifdef ZTS
#define FVAL_G(v) TSRMG(fval_globals_id, zend_fval_globals *, v)
#else
#define FVAL_G(v) (fval_globals.v)
#endif

#endif	/* PHP_FVAL_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
