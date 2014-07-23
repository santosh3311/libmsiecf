/*
 * Python bindings module for libmsiecf (pymsiecf)
 *
 * Copyright (C) 2009-2014, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <types.h>

#if defined( HAVE_STDLIB_H ) || defined( HAVE_WINAPI )
#include <stdlib.h>
#endif

#include "pymsiecf.h"
#include "pymsiecf_error.h"
#include "pymsiecf_file.h"
#include "pymsiecf_file_object_io_handle.h"
#include "pymsiecf_item.h"
#include "pymsiecf_item_flags.h"
#include "pymsiecf_item_types.h"
#include "pymsiecf_items.h"
#include "pymsiecf_leak.h"
#include "pymsiecf_libcerror.h"
#include "pymsiecf_libcstring.h"
#include "pymsiecf_libmsiecf.h"
#include "pymsiecf_python.h"
#include "pymsiecf_redirected.h"
#include "pymsiecf_unused.h"
#include "pymsiecf_url.h"
#include "pymsiecf_url_types.h"

#if !defined( LIBMSIECF_HAVE_BFIO )
LIBMSIECF_EXTERN \
int libmsiecf_check_file_signature_file_io_handle(
     libbfio_handle_t *file_io_handle,
     libmsiecf_error_t **error );
#endif

/* The pymsiecf module methods
 */
PyMethodDef pymsiecf_module_methods[] = {
	{ "get_version",
	  (PyCFunction) pymsiecf_get_version,
	  METH_NOARGS,
	  "get_version() -> String\n"
	  "\n"
	  "Retrieves the version." },

	{ "check_file_signature",
	  (PyCFunction) pymsiecf_check_file_signature,
	  METH_VARARGS | METH_KEYWORDS,
	  "check_file_signature(filename) -> Boolean\n"
	  "\n"
	  "Checks if a file has a MSIE Cache File (index.dat) file signature." },

	{ "check_file_signature_file_object",
	  (PyCFunction) pymsiecf_check_file_signature_file_object,
	  METH_VARARGS | METH_KEYWORDS,
	  "check_file_signature_file_object(file_object) -> Boolean\n"
	  "\n"
	  "Checks if a file has a MSIE Cache File (index.dat) file signature using a file-like object." },

	{ "open",
	  (PyCFunction) pymsiecf_file_new_open,
	  METH_VARARGS | METH_KEYWORDS,
	  "open(filename, mode='r') -> Object\n"
	  "\n"
	  "Opens a file." },

	{ "open_file_object",
	  (PyCFunction) pymsiecf_file_new_open_file_object,
	  METH_VARARGS | METH_KEYWORDS,
	  "open_file_object(file_object, mode='r') -> Object\n"
	  "\n"
	  "Opens a file using a file-like object." },

	/* Sentinel */
	{ NULL, NULL, 0, NULL }
};

/* Retrieves the pymsiecf/libmsiecf version
 * Returns a Python object if successful or NULL on error
 */
PyObject *pymsiecf_get_version(
           PyObject *self PYMSIECF_ATTRIBUTE_UNUSED,
           PyObject *arguments PYMSIECF_ATTRIBUTE_UNUSED )
{
	const char *errors           = NULL;
	const char *version_string   = NULL;
	size_t version_string_length = 0;

	PYMSIECF_UNREFERENCED_PARAMETER( self )
	PYMSIECF_UNREFERENCED_PARAMETER( arguments )

	Py_BEGIN_ALLOW_THREADS

	version_string = libmsiecf_get_version();

	Py_END_ALLOW_THREADS

	version_string_length = libcstring_narrow_string_length(
	                         version_string );

	/* Pass the string length to PyUnicode_DecodeUTF8
	 * otherwise it makes the end of string character is part
	 * of the string
	 */
	return( PyUnicode_DecodeUTF8(
	         version_string,
	         (Py_ssize_t) version_string_length,
	         errors ) );
}

#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )

/* Checks if the file has a MSIE Cache File (index.dat) file signature
 * Returns a Python object if successful or NULL on error
 */
PyObject *pymsiecf_check_file_signature(
           PyObject *self PYMSIECF_ATTRIBUTE_UNUSED,
           PyObject *arguments,
           PyObject *keywords )
{
	PyObject *exception_string    = NULL;
	PyObject *exception_traceback = NULL;
	PyObject *exception_type      = NULL;
	PyObject *exception_value     = NULL;
	PyObject *string_object       = NULL;
	libcerror_error_t *error      = NULL;
	static char *function         = "pymsiecf_check_file_signature";
	static char *keyword_list[]   = { "filename", NULL };
	const wchar_t *filename_wide  = NULL;
	const char *filename_narrow   = NULL;
	char *error_string            = NULL;
	int result                    = 0;

	PYMSIECF_UNREFERENCED_PARAMETER( self )

	/* Note that PyArg_ParseTupleAndKeywords with "s" will force Unicode strings to be converted to narrow character string.
	 * On Windows the narrow character strings contains an extended ASCII string with a codepage. Hence we get a conversion
	 * exception. We cannot use "u" here either since that does not allow us to pass non Unicode string objects and
	 * Python (at least 2.7) does not seems to automatically upcast them.
	 */
	if( PyArg_ParseTupleAndKeywords(
	     arguments,
	     keywords,
	     "|O",
	     keyword_list,
	     &string_object ) == 0 )
	{
		return( NULL );
	}
	PyErr_Clear();

	result = PyObject_IsInstance(
	          string_object,
	          (PyObject *) &PyUnicode_Type );

	if( result == -1 )
	{
		PyErr_Fetch(
		 &exception_type,
		 &exception_value,
		 &exception_traceback );

		exception_string = PyObject_Repr(
		                    exception_value );

		error_string = PyString_AsString(
		                exception_string );

		if( error_string != NULL )
		{
			PyErr_Format(
		         PyExc_RuntimeError,
			 "%s: unable to determine if string object is of type unicode with error: %s.",
			 function,
			 error_string );
		}
		else
		{
			PyErr_Format(
		         PyExc_RuntimeError,
			 "%s: unable to determine if string object is of type unicode.",
			 function );
		}
		Py_DecRef(
		 exception_string );

		return( NULL );
	}
	else if( result != 0 )
	{
		PyErr_Clear();

		filename_wide = (wchar_t *) PyUnicode_AsUnicode(
		                             string_object );
		Py_BEGIN_ALLOW_THREADS

		result = libmsiecf_check_file_signature_wide(
		          filename_wide,
		          &error );

		Py_END_ALLOW_THREADS

		if( result == -1 )
		{
			pymsiecf_error_raise(
			 error,
			 PyExc_IOError,
			 "%s: unable to check file signature.",
			 function );

			libcerror_error_free(
			 &error );

			return( NULL );
		}
		if( result != 0 )
		{
			return( Py_True );
		}
		return( Py_False );
	}
	PyErr_Clear();

	result = PyObject_IsInstance(
		  string_object,
		  (PyObject *) &PyString_Type );

	if( result == -1 )
	{
		PyErr_Fetch(
		 &exception_type,
		 &exception_value,
		 &exception_traceback );

		exception_string = PyObject_Repr(
				    exception_value );

		error_string = PyString_AsString(
				exception_string );

		if( error_string != NULL )
		{
			PyErr_Format(
		         PyExc_RuntimeError,
			 "%s: unable to determine if string object is of type string with error: %s.",
			 function,
			 error_string );
		}
		else
		{
			PyErr_Format(
		         PyExc_RuntimeError,
			 "%s: unable to determine if string object is of type string.",
			 function );
		}
		Py_DecRef(
		 exception_string );

		return( NULL );
	}
	else if( result != 0 )
	{
		PyErr_Clear();

		filename_narrow = PyString_AsString(
				   string_object );

		Py_BEGIN_ALLOW_THREADS

		result = libmsiecf_check_file_signature(
		          filename_narrow,
		          &error );

		Py_END_ALLOW_THREADS

		if( result == -1 )
		{
			pymsiecf_error_raise(
			 error,
			 PyExc_IOError,
			 "%s: unable to check file signature.",
			 function );

			libcerror_error_free(
			 &error );

			return( NULL );
		}
		if( result != 0 )
		{
			return( Py_True );
		}
		return( Py_False );
	}
	PyErr_Format(
	 PyExc_TypeError,
	 "%s: unsupported string object type",
	 function );

	return( NULL );
}

#else

/* Checks if the file has a MSIE Cache File (index.dat) file signature
 * Returns a Python object if successful or NULL on error
 */
PyObject *pymsiecf_check_file_signature(
           PyObject *self PYMSIECF_ATTRIBUTE_UNUSED,
           PyObject *arguments,
           PyObject *keywords )
{
	libcerror_error_t *error    = NULL;
	static char *function       = "pymsiecf_check_file_signature";
	static char *keyword_list[] = { "filename", NULL };
	const char *filename        = NULL;
	int result                  = 0;

	PYMSIECF_UNREFERENCED_PARAMETER( self )

	if( PyArg_ParseTupleAndKeywords(
	     arguments,
	     keywords,
	     "|s",
	     keyword_list,
	     &filename ) == 0 )
	{
		return( NULL );
	}
	Py_BEGIN_ALLOW_THREADS

	result = libmsiecf_check_file_signature(
	          filename,
	          &error );

	Py_END_ALLOW_THREADS

	if( result == -1 )
	{
		pymsiecf_error_raise(
		 error,
		 PyExc_IOError,
		 "%s: unable to check file signature.",
		 function );

		libcerror_error_free(
		 &error );

		return( NULL );
	}
	if( result != 0 )
	{
		return( Py_True );
	}
	return( Py_False );
}

#endif /* defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER ) */

/* Checks if the file has a MSIE Cache File (index.dat) file signature using a file-like object
 * Returns a Python object if successful or NULL on error
 */
PyObject *pymsiecf_check_file_signature_file_object(
           PyObject *self PYMSIECF_ATTRIBUTE_UNUSED,
           PyObject *arguments,
           PyObject *keywords )
{
	libcerror_error_t *error         = NULL;
	libbfio_handle_t *file_io_handle = NULL;
	PyObject *file_object            = NULL;
	static char *function            = "pymsiecf_check_file_signature_file_object";
	static char *keyword_list[]      = { "file_object", NULL };
	int result                       = 0;

	PYMSIECF_UNREFERENCED_PARAMETER( self )

	if( PyArg_ParseTupleAndKeywords(
	     arguments,
	     keywords,
	     "|O",
	     keyword_list,
	     &file_object ) == 0 )
	{
		return( NULL );
	}
	if( pymsiecf_file_object_initialize(
	     &file_io_handle,
	     file_object,
	     &error ) != 1 )
	{
		pymsiecf_error_raise(
		 error,
		 PyExc_MemoryError,
		 "%s: unable to initialize file IO handle.",
		 function );

		libcerror_error_free(
		 &error );

		goto on_error;
	}
	Py_BEGIN_ALLOW_THREADS

	result = libmsiecf_check_file_signature_file_io_handle(
	          file_io_handle,
	          &error );

	Py_END_ALLOW_THREADS

	if( result == -1 )
	{
		pymsiecf_error_raise(
		 error,
		 PyExc_IOError,
		 "%s: unable to check file signature.",
		 function );

		libcerror_error_free(
		 &error );

		goto on_error;
	}
	if( libbfio_handle_free(
	     &file_io_handle,
	     &error ) != 1 )
	{
		pymsiecf_error_raise(
		 error,
		 PyExc_MemoryError,
		 "%s: unable to free file IO handle.",
		 function );

		libcerror_error_free(
		 &error );

		goto on_error;
	}
	if( result != 0 )
	{
		return( Py_True );
	}
	return( Py_False );

on_error:
	if( file_io_handle != NULL )
	{
		libbfio_handle_free(
		 &file_io_handle,
		 NULL );
	}
	return( NULL );
}

/* Declarations for DLL import/export
 */
#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif

/* Initializes the pymsiecf module
 */
PyMODINIT_FUNC initpymsiecf(
                void )
{
	PyObject *module                     = NULL;
	PyTypeObject *file_type_object       = NULL;
	PyTypeObject *item_type_object       = NULL;
	PyTypeObject *item_flags_type_object = NULL;
	PyTypeObject *item_types_type_object = NULL;
	PyTypeObject *items_type_object      = NULL;
	PyTypeObject *leak_type_object       = NULL;
	PyTypeObject *redirected_type_object = NULL;
	PyTypeObject *url_type_object        = NULL;
	PyTypeObject *url_types_type_object  = NULL;
	PyGILState_STATE gil_state           = 0;

	/* Create the module
	 * This function must be called before grabbing the GIL
	 * otherwise the module will segfault on a version mismatch
	 */
	module = Py_InitModule3(
	          "pymsiecf",
	          pymsiecf_module_methods,
	          "Python libmsiecf module (pymsiecf)." );

	PyEval_InitThreads();

	gil_state = PyGILState_Ensure();

	/* Setup the file type object
	 */
	pymsiecf_file_type_object.tp_new = PyType_GenericNew;

	if( PyType_Ready(
	     &pymsiecf_file_type_object ) < 0 )
	{
		goto on_error;
	}
	Py_IncRef(
	 (PyObject *) &pymsiecf_file_type_object );

	file_type_object = &pymsiecf_file_type_object;

	PyModule_AddObject(
	 module,
	 "file",
	 (PyObject *) file_type_object );

	/* Setup the item type object
	 */
	pymsiecf_item_type_object.tp_new = PyType_GenericNew;

	if( PyType_Ready(
	     &pymsiecf_item_type_object ) < 0 )
	{
		goto on_error;
	}
	Py_IncRef(
	 (PyObject *) &pymsiecf_item_type_object );

	item_type_object = &pymsiecf_item_type_object;

	PyModule_AddObject(
	 module,
	 "item",
	 (PyObject *) item_type_object );

	/* Setup the items type object
	 */
	pymsiecf_items_type_object.tp_new = PyType_GenericNew;

	if( PyType_Ready(
	     &pymsiecf_items_type_object ) < 0 )
	{
		goto on_error;
	}
	Py_IncRef(
	 (PyObject *) &pymsiecf_items_type_object );

	items_type_object = &pymsiecf_items_type_object;

	PyModule_AddObject(
	 module,
	 "_items",
	 (PyObject *) items_type_object );

	/* Setup the leak type object
	 */
	pymsiecf_leak_type_object.tp_new = PyType_GenericNew;

	if( PyType_Ready(
	     &pymsiecf_leak_type_object ) < 0 )
	{
		goto on_error;
	}
	Py_IncRef(
	 (PyObject *) &pymsiecf_leak_type_object );

	leak_type_object = &pymsiecf_leak_type_object;

	PyModule_AddObject(
	 module,
	 "leak",
	 (PyObject *) leak_type_object );

	/* Setup the redirected type object
	 */
	pymsiecf_redirected_type_object.tp_new = PyType_GenericNew;

	if( PyType_Ready(
	     &pymsiecf_redirected_type_object ) < 0 )
	{
		goto on_error;
	}
	Py_IncRef(
	 (PyObject *) &pymsiecf_redirected_type_object );

	redirected_type_object = &pymsiecf_redirected_type_object;

	PyModule_AddObject(
	 module,
	 "redirected",
	 (PyObject *) redirected_type_object );

	/* Setup the URL type object
	 */
	pymsiecf_url_type_object.tp_new = PyType_GenericNew;

	if( PyType_Ready(
	     &pymsiecf_url_type_object ) < 0 )
	{
		goto on_error;
	}
	Py_IncRef(
	 (PyObject *) &pymsiecf_url_type_object );

	url_type_object = &pymsiecf_url_type_object;

	PyModule_AddObject(
	 module,
	 "url",
	 (PyObject *) url_type_object );

	/* Setup the item types type object
	 */
	pymsiecf_item_types_type_object.tp_new = PyType_GenericNew;

	if( pymsiecf_item_types_init_type(
	     &pymsiecf_item_types_type_object ) != 1 )
	{
		goto on_error;
	}
	if( PyType_Ready(
	     &pymsiecf_item_types_type_object ) < 0 )
	{
		goto on_error;
	}
	Py_IncRef(
	 (PyObject *) &pymsiecf_item_types_type_object );

	item_types_type_object = &pymsiecf_item_types_type_object;

	PyModule_AddObject(
	 module,
	 "item_types",
	 (PyObject *) item_types_type_object );

	/* Setup the item flags type object
	 */
	pymsiecf_item_flags_type_object.tp_new = PyType_GenericNew;

	if( pymsiecf_item_flags_init_type(
	     &pymsiecf_item_flags_type_object ) != 1 )
	{
		goto on_error;
	}
	if( PyType_Ready(
	     &pymsiecf_item_flags_type_object ) < 0 )
	{
		goto on_error;
	}
	Py_IncRef(
	 (PyObject *) &pymsiecf_item_flags_type_object );

	item_flags_type_object = &pymsiecf_item_flags_type_object;

	PyModule_AddObject(
	 module,
	 "item_flags",
	 (PyObject *) item_flags_type_object );

	/* Setup the URL types type object
	 */
	pymsiecf_url_types_type_object.tp_new = PyType_GenericNew;

	if( pymsiecf_url_types_init_type(
	     &pymsiecf_url_types_type_object ) != 1 )
	{
		goto on_error;
	}
	if( PyType_Ready(
	     &pymsiecf_url_types_type_object ) < 0 )
	{
		goto on_error;
	}
	Py_IncRef(
	 (PyObject *) &pymsiecf_url_types_type_object );

	url_types_type_object = &pymsiecf_url_types_type_object;

	PyModule_AddObject(
	 module,
	 "url_types",
	 (PyObject *) url_types_type_object );

on_error:
	PyGILState_Release(
	 gil_state );
}

