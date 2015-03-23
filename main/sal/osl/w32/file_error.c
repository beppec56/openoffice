/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#define UNICODE
#define _UNICODE
#define _WIN32_WINNT_0x0500
#include "systools/win32/uwinapi.h"

#include "file_error.h"

#include "osl/diagnose.h"
#include "osl/thread.h"

/* OS error to oslFileError values mapping table */
struct osl_file_error_entry
{
	unsigned long oscode; /* OS return value */
	int errnocode;        /* oslFileError code */
};

static const struct osl_file_error_entry errtable[] = {
  {  ERROR_SUCCESS,				   osl_File_E_None     },  /* 0 */
  {  ERROR_INVALID_FUNCTION,       osl_File_E_INVAL    },  /* 1 */
  {  ERROR_FILE_NOT_FOUND,         osl_File_E_NOENT    },  /* 2 */
  {  ERROR_PATH_NOT_FOUND,         osl_File_E_NOENT    },  /* 3 */
  {  ERROR_TOO_MANY_OPEN_FILES,    osl_File_E_MFILE    },  /* 4 */
  {  ERROR_ACCESS_DENIED,          osl_File_E_ACCES    },  /* 5 */
  {  ERROR_INVALID_HANDLE,         osl_File_E_BADF     },  /* 6 */
  {  ERROR_ARENA_TRASHED,          osl_File_E_NOMEM    },  /* 7 */
  {  ERROR_NOT_ENOUGH_MEMORY,      osl_File_E_NOMEM    },  /* 8 */
  {  ERROR_INVALID_BLOCK,          osl_File_E_NOMEM    },  /* 9 */
  {  ERROR_BAD_ENVIRONMENT,        osl_File_E_2BIG     },  /* 10 */
  {  ERROR_BAD_FORMAT,             osl_File_E_NOEXEC   },  /* 11 */
  {  ERROR_INVALID_ACCESS,         osl_File_E_INVAL    },  /* 12 */
  {  ERROR_INVALID_DATA,           osl_File_E_INVAL    },  /* 13 */
  {  ERROR_INVALID_DRIVE,          osl_File_E_NOENT    },  /* 15 */
  {  ERROR_CURRENT_DIRECTORY,      osl_File_E_ACCES    },  /* 16 */
  {  ERROR_NOT_SAME_DEVICE,        osl_File_E_XDEV     },  /* 17 */
  {  ERROR_NO_MORE_FILES,          osl_File_E_NOENT    },  /* 18 */
  {  ERROR_NOT_READY,              osl_File_E_NOTREADY },  /* 21 */
  {  ERROR_LOCK_VIOLATION,         osl_File_E_ACCES    },  /* 33 */
  {  ERROR_BAD_NETPATH,            osl_File_E_NOENT    },  /* 53 */
  {  ERROR_NETWORK_ACCESS_DENIED,  osl_File_E_ACCES    },  /* 65 */
  {  ERROR_BAD_NET_NAME,           osl_File_E_NOENT    },  /* 67 */
  {  ERROR_FILE_EXISTS,            osl_File_E_EXIST    },  /* 80 */
  {  ERROR_CANNOT_MAKE,            osl_File_E_ACCES    },  /* 82 */
  {  ERROR_FAIL_I24,               osl_File_E_ACCES    },  /* 83 */
  {  ERROR_INVALID_PARAMETER,      osl_File_E_INVAL    },  /* 87 */
  {  ERROR_NO_PROC_SLOTS,          osl_File_E_AGAIN    },  /* 89 */
  {  ERROR_DRIVE_LOCKED,           osl_File_E_ACCES    },  /* 108 */
  {  ERROR_BROKEN_PIPE,            osl_File_E_PIPE     },  /* 109 */
  {  ERROR_DISK_FULL,              osl_File_E_NOSPC    },  /* 112 */
  {  ERROR_INVALID_TARGET_HANDLE,  osl_File_E_BADF     },  /* 114 */
  {  ERROR_INVALID_HANDLE,         osl_File_E_INVAL    },  /* 124 */
  {  ERROR_WAIT_NO_CHILDREN,       osl_File_E_CHILD    },  /* 128 */
  {  ERROR_CHILD_NOT_COMPLETE,     osl_File_E_CHILD    },  /* 129 */
  {  ERROR_DIRECT_ACCESS_HANDLE,   osl_File_E_BADF     },  /* 130 */
  {  ERROR_NEGATIVE_SEEK,          osl_File_E_INVAL    },  /* 131 */
  {  ERROR_SEEK_ON_DEVICE,         osl_File_E_ACCES    },  /* 132 */
  {  ERROR_DIR_NOT_EMPTY,          osl_File_E_NOTEMPTY },  /* 145 */
  {  ERROR_NOT_LOCKED,             osl_File_E_ACCES    },  /* 158 */
  {  ERROR_BAD_PATHNAME,           osl_File_E_NOENT    },  /* 161 */
  {  ERROR_MAX_THRDS_REACHED,      osl_File_E_AGAIN    },  /* 164 */
  {  ERROR_LOCK_FAILED,            osl_File_E_ACCES    },  /* 167 */
  {  ERROR_ALREADY_EXISTS,         osl_File_E_EXIST    },  /* 183 */
  {  ERROR_FILENAME_EXCED_RANGE,   osl_File_E_NOENT    },  /* 206 */
  {  ERROR_NESTING_NOT_ALLOWED,    osl_File_E_AGAIN    },  /* 215 */
  {  ERROR_DIRECTORY,              osl_File_E_NOENT    },  /* 267 */
  {  ERROR_NOT_ENOUGH_QUOTA,       osl_File_E_NOMEM    },  /* 1816 */
  {  ERROR_UNEXP_NET_ERR,          osl_File_E_NETWORK  }   /* 59 */
};
    
/* The following two constants must be the minimum and maximum
   values in the (contiguous) range of osl_File_E_xec Failure errors.
*/
#define MIN_EXEC_ERROR ERROR_INVALID_STARTING_CODESEG
#define MAX_EXEC_ERROR ERROR_INFLOOP_IN_RELOC_CHAIN

/* These are the low and high value in the range of errors that are
   access violations
*/
#define MIN_EACCES_RANGE ERROR_WRITE_PROTECT
#define MAX_EACCES_RANGE ERROR_SHARING_BUFFER_EXCEEDED

oslFileError oslTranslateFileError (/*DWORD*/ unsigned long dwError)
{
	static const int n = sizeof(errtable)/sizeof(errtable[0]);

	int i;
    OSL_LOG_TRACE_FUNCTION( "oslTranslateFileError", __LINE__, "dwError %08x ", dwError );
	for (i = 0; i < n; ++i )
	{
		if (dwError == errtable[i].oscode)
			return (oslFileError)(errtable[i].errnocode);
	}

	/* The error code wasn't in the table.  We check for a range of 
	   osl_File_E_ACCES errors or exec failure errors (ENOEXEC).  
	   Otherwise osl_File_E_INVAL is returned.
	*/
    OSL_LOG_TRACE_FUNCTION( "oslTranslateFileError", __LINE__, "The error code wasn't in the table: dwError %08x ", dwError );
	if ( (dwError >= MIN_EACCES_RANGE) && (dwError <= MAX_EACCES_RANGE) )
		return osl_File_E_ACCES;
	else if ( (dwError >= MIN_EXEC_ERROR) && (dwError <= MAX_EXEC_ERROR) )
		return osl_File_E_NOEXEC;
	else
		return osl_File_E_INVAL;
}

//##################################################### 
#if OSL_DEBUG_LEVEL > 0
void _osl_warnFile( const char *message, rtl_uString *ustrFile )
{
	char szBuffer[2048];

	if (ustrFile)
	{
		rtl_String	*strFile = NULL;

		rtl_uString2String( &strFile, rtl_uString_getStr( ustrFile ), rtl_uString_getLength( ustrFile ),
							osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );
		snprintf( szBuffer, sizeof(szBuffer), message, strFile->buffer );
		rtl_string_release( strFile );

		message = szBuffer;
	}
	OSL_ENSURE( 0, message );
}
#endif /* OSL_DEBUG_LEVEL */
