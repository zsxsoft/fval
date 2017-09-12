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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "SAPI.h"
#include "zend_compile.h"
#include "zend_execute.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "../php_fval.h"

ZEND_DECLARE_MODULE_GLOBALS(fval)

/* {{{ fval_functions[]
*/
zend_function_entry fval_functions[] = {
	{NULL, NULL, NULL}
};
/* }}} */

/** {{{ module depends
*/
zend_module_dep fval_deps[] = {
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ fval_module_entry
*/
zend_module_entry fval_module_entry = {
	STANDARD_MODULE_HEADER_EX, NULL,
	fval_deps,
	"fval",
	fval_functions,
	PHP_MINIT(fval),
	PHP_MSHUTDOWN(fval),
	PHP_RINIT(fval),
	PHP_RSHUTDOWN(fval),
	PHP_MINFO(fval),
	PHP_FVAL_VERSION,
	PHP_MODULE_GLOBALS(fval),
	NULL,
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

static inline int fval_check_wrote_files (zval *file, zend_string **out) /* {{{ */ {
	zend_string *resolved_path = zend_resolve_path(Z_STRVAL_P(file), Z_STRLEN_P(file));
	if (!resolved_path) {
		resolved_path = zend_string_copy(Z_STR_P(file));
	}
	if (out != NULL) *out = resolved_path;
	return zend_hash_exists(&FVAL_G(fval_wrote_files), resolved_path);
} /* }}} */

static void fval_register_wrote_files (zval* file) /* {{{ */ {
	zend_string* resolved_path;
	fval_check_wrote_files(file, &resolved_path);
	zend_hash_add_empty_element(&FVAL_G(fval_wrote_files), resolved_path);
	zend_string_release(resolved_path);
} /* }}} */

static zval *php_fval_get_zval_ptr_tmpvar(zend_execute_data *execute_data, uint32_t var, zend_free_op *should_free) /* {{{ */ {
	zval *ret = EX_VAR(var);

	if (should_free) {
		*should_free = ret;
	}
	ZVAL_DEREF(ret);

	return ret;
}
/* }}} */

#ifndef CV_DEF_OF
#define CV_DEF_OF(i) (EX(func)->op_array.vars[i])
#endif

static zval *php_fval_get_zval_ptr_cv(zend_execute_data *execute_data, uint32_t var, int type, int force_ret) /* {{{ */ {
	zval *ret = EX_VAR(var);

	if (UNEXPECTED(Z_TYPE_P(ret) == IS_UNDEF)) {
		if (force_ret) {
			switch (type) {
				case BP_VAR_R:
				case BP_VAR_UNSET:
					zend_error(E_NOTICE, "Undefined variable: %s", ZSTR_VAL(CV_DEF_OF(EX_VAR_TO_NUM(var))));
				case BP_VAR_IS:
					ret = &EG(uninitialized_zval);
					break;
				case BP_VAR_RW:
					zend_error(E_NOTICE, "Undefined variable: %s", ZSTR_VAL(CV_DEF_OF(EX_VAR_TO_NUM(var))));
				case BP_VAR_W:
					ZVAL_NULL(ret);
					break;
			}
		} else {
			return NULL;
		}
	} else {
		ZVAL_DEREF(ret);
	}
	return ret;
}
/* }}} */

static zval *php_fval_get_zval_ptr(zend_execute_data *execute_data, int op_type, znode_op op, fval_free_op *should_free, int type, int force_ret) /* {{{ */ {
	if (op_type & (IS_TMP_VAR|IS_VAR)) {
		return php_fval_get_zval_ptr_tmpvar(execute_data, op.var, should_free);
	} else {
		*should_free = NULL;
		if (op_type == IS_CONST) {
			return EX_CONSTANT(op);
		} else if (op_type == IS_CV) {
			return php_fval_get_zval_ptr_cv(execute_data, op.var, type, force_ret);
		} else {
			return NULL;
		}
	}
}
/* }}} */

static zval *php_fval_get_zval_ptr_ptr_var(zend_execute_data *execute_data, uint32_t var, zend_free_op *should_free) /* {{{ */ {
	zval *ret = EX_VAR(var);

	if (EXPECTED(Z_TYPE_P(ret) == IS_INDIRECT)) {
		*should_free = NULL;
		ret = Z_INDIRECT_P(ret);
	} else {
		*should_free = ret;
	}
	return ret;
}
/* }}} */

static zval *php_fval_get_zval_ptr_ptr(zend_execute_data *execute_data, int op_type, znode_op op, fval_free_op *should_free, int type) /* {{{ */ {
	if (op_type == IS_CV) {
		*should_free = NULL;
		return php_fval_get_zval_ptr_cv(execute_data, op.var, type, 1);
	} else if (op_type == IS_VAR) {
		ZEND_ASSERT(op_type == IS_VAR);
		return php_fval_get_zval_ptr_ptr_var(execute_data, op.var, should_free);
	} else if (op_type == IS_UNUSED) {
		*should_free = NULL;
		return &EX(This);
	} else {
		ZEND_ASSERT(0);
	}
}
/* }}} */

static void php_fval_error(const char *fname, const char *format, ...) /* {{{ */ {
	char *buffer, *msg;
	va_list args;

	va_start(args, format);
	vspprintf(&buffer, 0, format, args);
	spprintf(&msg, 0, "%s() [%s]: %s", get_active_function_name(), fname, buffer);
	efree(buffer);
	zend_error(FVAL_G(error_level), msg);
	efree(msg);
	va_end(args);
} /* }}} */

static void php_fval_fatal_error(const char *fname, const char *format, ...) /* {{{ */ {
	char *buffer, *msg;
	va_list args;
	// Force ignore "@"
	// zend_executed.c, kind == ZEND_LIVE_SILENCE
	EG(error_reporting) = 1;

	va_start(args, format);
	vspprintf(&buffer, 0, format, args);
	spprintf(&msg, 0, "%s() [%s]: %s", get_active_function_name(), fname, buffer);
	efree(buffer);
	zend_error(1, msg);
	efree(msg);
	va_end(args);
} /* }}} */

static int php_fval_init_dynamic_fcall_handler(zend_execute_data *execute_data) /* {{{ */ {
	const zend_op *opline = execute_data->opline;
	fval_free_op free_op2;
	zval *op2;

	op2 = php_fval_get_zval_ptr(execute_data, opline->op2_type, opline->op2, &free_op2, BP_VAR_R, 0);

	if (op2) {
		if (IS_STRING == Z_TYPE_P(op2)) {
			if (FVAL_POSSIBLE(Z_STR_P(op2))) {
				php_fval_error("fcall", "Attempt to call a function which name might unsafe");
			}
		} else if (IS_ARRAY == Z_TYPE_P(op2)) {
			zval *cname = zend_hash_index_find(Z_ARRVAL_P(op2), 0);
			zval *mname = zend_hash_index_find(Z_ARRVAL_P(op2), 0);

			if (cname && IS_STRING == Z_TYPE_P(cname) && FVAL_POSSIBLE(Z_STR_P(cname))) {
				php_fval_error("fcall", "Attempt to call a method of a class which name might unsafe");
			} else if (mname && IS_STRING == Z_TYPE_P(mname) && FVAL_POSSIBLE(Z_STR_P(mname))) {
				php_fval_error("fcall", "Attempt to call a method which name might unsafe");
			}
		}
	}

	return ZEND_USER_OPCODE_DISPATCH;
} /* }}} */

static int php_fval_include_or_eval_handler(zend_execute_data *execute_data) /* {{{ */ {
	const zend_op *opline = execute_data->opline;
	fval_free_op free_op1;
	zval *op1;

	op1 = php_fval_get_zval_ptr(execute_data, opline->op1_type, opline->op1, &free_op1, BP_VAR_R, 0);

	if (FVAL_G(disable_eval) && opline->extended_value == ZEND_EVAL) {
		php_fval_fatal_error("eval", "Disabled function for security reasons.");
	}

	if (op1 && IS_STRING == Z_TYPE_P(op1)) {
		if (fval_check_wrote_files(op1, NULL)) {
			php_fval_fatal_error("include_or_require", "Including wrote file: %s", Z_STRVAL_P(op1));
		}
	}
	return ZEND_USER_OPCODE_DISPATCH;
} /* }}} */

static void php_fval_fcall_check(zend_execute_data *ex, const zend_op *opline, zend_function *fbc) /* {{{ */ {
	int arg_count = ZEND_CALL_NUM_ARGS(ex);

	if (!arg_count) {
		return;
	}

	if (fbc->common.scope == NULL) {
		zend_string *fname = fbc->common.function_name;
			if (zend_string_equals_literal(fname, "passthru") ||
				zend_string_equals_literal(fname, "system") ||
				zend_string_equals_literal(fname, "exec") ||
				zend_string_equals_literal(fname, "shell_exec") ||
				zend_string_equals_literal(fname, "proc_open") ||
				zend_string_equals_literal(fname, "popen")) {
					if (FVAL_G(disable_functions)) {
						php_fval_fatal_error(ZSTR_VAL(fname), "Disabled function for security reasons.");
					}
			} else if (zend_string_equals_literal(fname, "file_put_contents")) {
				if (FVAL_G(disable_eval)) {
					zval *fp = ZEND_CALL_ARG(ex, 1);
					if (IS_STRING == Z_TYPE_P(fp)) {
						fval_register_wrote_files(fp);
					}
				}
			} else if (
				zend_string_equals_literal(fname, "move_uploaded_file") ||
				zend_string_equals_literal(fname, "copy")
				) {
				if (FVAL_G(disable_eval)) {
					zval *fp = ZEND_CALL_ARG(ex, 2);
					if (IS_STRING == Z_TYPE_P(fp)) {
						fval_register_wrote_files(fp);
					}
				}
			} else if (zend_string_equals_literal(fname, "fopen")) {
				if (FVAL_G(disable_eval)) {
					zval *fp = ZEND_CALL_ARG(ex, 1);
					zval *mode_zval = ZEND_CALL_ARG(ex, 2);
					if (IS_STRING == Z_TYPE_P(fp) && IS_STRING == Z_TYPE_P(mode_zval)) {
						// w = Open for write
						// a = Open for append
						// c = Open for writing only
						// x = Create and open for writing
						char* mode = Z_STRVAL_P(mode_zval);
						size_t mode_length = Z_STRLEN_P(mode_zval);
						for (int i = 0; i < mode_length; i++) {
							if ((mode[i] == 'w' || mode[i] == 'W') ||
								(mode[i] == 'a' || mode[i] == 'A') ||
								(mode[i] == 'c' || mode[i] == 'C') ||
								(mode[i] == 'x' || mode[i] == 'X')) {
								fval_register_wrote_files(fp);
								break;
							}
						}
					}
				}
			}
	}
} /* }}} */

static int php_fval_fcall_handler(zend_execute_data *execute_data) /* {{{ */ {
	const zend_op *opline = execute_data->opline;
	zend_execute_data *call = execute_data->call;
	zend_function *fbc = call->func;

	if (fbc->type == ZEND_INTERNAL_FUNCTION) {
		php_fval_fcall_check(call, opline, fbc);
	}

	return ZEND_USER_OPCODE_DISPATCH;
} /* }}} */

static void php_fval_register_handlers() /* {{{ */ {
	// zend_set_user_opcode_handler(ZEND_INIT_USER_CALL, php_fval_init_dynamic_fcall_handler);
	// zend_set_user_opcode_handler(ZEND_INIT_DYNAMIC_CALL, php_fval_init_dynamic_fcall_handler);
	zend_set_user_opcode_handler(ZEND_INCLUDE_OR_EVAL, php_fval_include_or_eval_handler);
	zend_set_user_opcode_handler(ZEND_DO_FCALL, php_fval_fcall_handler);
	zend_set_user_opcode_handler(ZEND_DO_ICALL, php_fval_fcall_handler);
	zend_set_user_opcode_handler(ZEND_DO_FCALL_BY_NAME, php_fval_fcall_handler);
} /* }}} */

#ifdef COMPILE_DL_FVAL
ZEND_GET_MODULE(fval)
#endif

static PHP_INI_MH(OnUpdateErrorLevel) /* {{{ */ {
	if (!new_value) {
		FVAL_G(error_level) = E_USER_WARNING;
	} else {
		FVAL_G(error_level) = (int)atoi(ZSTR_VAL(new_value));
	}
	return SUCCESS;
} /* }}} */

/* {{{ PHP_INI
*/
PHP_INI_BEGIN()
	STD_PHP_INI_BOOLEAN("fval.enable", "1", PHP_INI_SYSTEM, OnUpdateBool, enable, zend_fval_globals, fval_globals)
	STD_PHP_INI_BOOLEAN("fval.disable_functions", "1", PHP_INI_SYSTEM, OnUpdateBool, disable_functions, zend_fval_globals, fval_globals)
	STD_PHP_INI_BOOLEAN("fval.disable_eval", "1", PHP_INI_SYSTEM, OnUpdateBool, disable_eval, zend_fval_globals, fval_globals)
	STD_PHP_INI_ENTRY("fval.error_level", "512", PHP_INI_ALL, OnUpdateErrorLevel, error_level, zend_fval_globals, fval_globals)
PHP_INI_END()
	/* }}} */

/* {{{ PHP_MINIT_FUNCTION
*/
PHP_MINIT_FUNCTION(fval)
{
	REGISTER_INI_ENTRIES();

	if (!FVAL_G(enable)) {
		return SUCCESS;
	}

	// Writed files list should be kept with the process.
	zend_hash_init(&FVAL_G(fval_wrote_files), 8, NULL, NULL, 0);
	php_fval_register_handlers();

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(fval)
{
	UNREGISTER_INI_ENTRIES();
	// zend_hash_destroy(&FVAL_G(fval_wrote_files));
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
*/
PHP_RINIT_FUNCTION(fval)
{
	if (SG(sapi_started) || !FVAL_G(enable)) {
		return SUCCESS;
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
*/
PHP_RSHUTDOWN_FUNCTION(fval)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(fval)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "fval support", "enabled");
	php_info_print_table_row(2, "Version", PHP_FVAL_VERSION);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
