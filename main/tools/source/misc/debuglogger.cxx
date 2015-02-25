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

#include "precompiled_tools.hxx"

#include "sal/config.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <stdio.h> // snprintf, vsnprintf


//#include <rtl/uri.hxx>
 //#include <rtl/ustrbuf.hxx>
 //#include <com/sun/star/uno/Reference.h>


#include <vector>

#include "rtl/string.hxx"
#include "rtl/bootstrap.hxx"
#include "osl/diagnose.h"
#include "osl/time.h"
#include "osl/file.h"
#include "osl/module.h"
#include "osl/mutex.hxx"
#include "osl/process.h"
#include "osl/thread.hxx"
#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/logging/XDebugLogRing.hpp>

#include "tools/debuglogger.hxx"

using namespace ::com::sun::star;

namespace tools
{
    static uno::Reference< logging::XDebugLogRing > m_xDebugLogRing;
    static osl::Mutex   aMutex;

    static pfunc_osl_printDebugMessage pOriginal_osl_DebugMessageFunc = NULL;

    static void sendLoggerLineMessage(char const* pszMessage)
    {
        if (! m_xDebugLogRing.is() )
        {
            try
            {
                ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
                if ( aContext.is() )
                    m_xDebugLogRing.set( aContext.getSingleton( "com.sun.star.logging.SingleDebugLogRing" ), uno::UNO_QUERY_THROW );
            }
            catch( uno::Exception& )
            { }
        }

        if ( m_xDebugLogRing.is() )
        {
            rtl::OUString aMessage(pszMessage, std::strlen(pszMessage), RTL_TEXTENCODING_ASCII_US);
            m_xDebugLogRing->logString( aMessage );
        }
    }

    static void vaListLogFunction( const sal_Char * funcName, sal_Int32 lineNumber, const sal_Char* pszFormat, va_list args)
    {
        oslDateTime currentDate;
        TimeValue   currentTime;
        osl_getSystemTime(&currentTime);
        osl_getDateTimeFromTimeValue( &currentTime, &currentDate );

        osl::MutexGuard aGuard( aMutex );

        char buf[8192];
        int n1 = snprintf( buf, sizeof buf, "%02d:%02d:%02d pid:%u/%" SAL_PRIuUINT32 ": \"%s:%d\n - ",
                           currentDate.Hours, currentDate.Minutes, currentDate.Seconds,
                           osl_getpidDebugLog(), osl::Thread::getCurrentIdentifier(),
                           funcName,lineNumber );
        OSL_ASSERT(n1 >= 0 && (static_cast< unsigned int >(n1) < sizeof buf - RTL_CONSTASCII_LENGTH("\"...\n")));
        int n2 = sizeof buf  - n1 - RTL_CONSTASCII_LENGTH("\"...\n");
        int n3 = vsnprintf(buf + n1, n2, pszFormat, args);
        if (n3 < 0) {
            std::strcpy(buf+n1, "\"???\n");
        } else if (n3 < n2) {
            std::strcpy(buf + n1 + n3, "\"\n");
        } else {
            std::strcpy(buf + n1 + n2 - 1, "\"...\n");
        }
        sendLoggerLineMessage(buf);
        
    }

    void SAL_CALL debugPrint( const sal_Char * pszMessage )
    {

        if (! m_xDebugLogRing.is() )
        {
            try
            {
                ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
                if ( aContext.is() )
                    m_xDebugLogRing.set( aContext.getSingleton( "com.sun.star.logging.SingleDebugLogRing" ), uno::UNO_QUERY_THROW );
            }
            catch( uno::Exception& )
            { }
        }

        if ( m_xDebugLogRing.is() )
        {
            rtl::OUString aMessage(pszMessage, std::strlen(pszMessage), RTL_TEXTENCODING_ASCII_US);
            m_xDebugLogRing->logString( aMessage );
        }
    }

    void initDebugLog()
    {
        osl_setLogMessageFunc(vaListLogFunction);
    }

    void addExceptionErrorDebugLog(const sal_Char* pszFileName, sal_Int32 nLine, const sal_Char* pszMessage)
    {
        initDebugLog();
        osl::MutexGuard aGuard( aMutex );
        pOriginal_osl_DebugMessageFunc = osl_setDebugMessageFuncDebugLog(debugPrint);
        osl_assertFailedLineDebugLog( pszFileName, nLine, pszMessage );
        osl_setDebugMessageFuncDebugLog(debugPrint);
        writeDebugLog("main_tools_source_misc_debuglogger-exception.log");
    }

    void addDebugLogExc(char const* funcName,  const int lineNumber, char const* pszFormat, ... )
    {
        initDebugLog();
        const char* starting="\n======vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv========\nError: ";
        oslDateTime currentDate;
        TimeValue   currentTime;
        osl_getSystemTime(&currentTime);
        osl_getDateTimeFromTimeValue( &currentTime, &currentDate );

        osl::MutexGuard aGuard( aMutex );

        va_list args;
        va_start(args, pszFormat);
        char buf[8192];
        int n1 = snprintf( buf, sizeof buf, "%s%02d:%02d:%02d pid:%u/%" SAL_PRIuUINT32 ": \"%s:%d\n - ",
                           starting,
                           currentDate.Hours, currentDate.Minutes, currentDate.Seconds,
                           osl_getpidDebugLog(), osl::Thread::getCurrentIdentifier(),
                           funcName,lineNumber );
        OSL_ASSERT(n1 >= 0 && (static_cast< unsigned int >(n1) < sizeof buf -
                               RTL_CONSTASCII_LENGTH(
                                   "\"...\n========^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^========\n")));
        int n2 = sizeof buf  - n1 -
            RTL_CONSTASCII_LENGTH(
                "\"...\n========^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^========\n");
        int n3 = vsnprintf(buf + n1, n2, pszFormat, args);
        if (n3 < 0) {
            std::strcpy(buf+n1,
                        "\"???\n========^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^========\n");
        } else if (n3 < n2) {
            std::strcpy(buf + n1 + n3,
                        "\"\n========^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^========\n");
        } else {
            std::strcpy(buf + n1 + n2 - 1,
                        "\"...\n========^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^========\n");
        }
        va_end(args);
        sendLoggerLineMessage(buf);

    }

    void addDebugLogFunction(char const* funcName,  const int lineNumber, char const* pszFormat, ... )
    {
        initDebugLog();
        oslDateTime currentDate;
        TimeValue   currentTime;
        osl_getSystemTime(&currentTime);
        osl_getDateTimeFromTimeValue( &currentTime, &currentDate );

        osl::MutexGuard aGuard( aMutex );

        va_list args;
        va_start(args, pszFormat);
        char buf[8192];
        int n1 = snprintf( buf, sizeof buf, "%02d:%02d:%02d pid:%u/%" SAL_PRIuUINT32 ": \"%s:%d\n - ",
                           currentDate.Hours, currentDate.Minutes, currentDate.Seconds,
                           osl_getpidDebugLog(), osl::Thread::getCurrentIdentifier(),
                           funcName,lineNumber );
        OSL_ASSERT(n1 >= 0 && (static_cast< unsigned int >(n1) < sizeof buf - RTL_CONSTASCII_LENGTH("\"...\n")));
        int n2 = sizeof buf  - n1 - RTL_CONSTASCII_LENGTH("\"...\n");
        int n3 = vsnprintf(buf + n1, n2, pszFormat, args);
        if (n3 < 0) {
            std::strcpy(buf+n1, "\"???\n");
        } else if (n3 < n2) {
            std::strcpy(buf + n1 + n3, "\"\n");
        } else {
            std::strcpy(buf + n1 + n2 - 1, "\"...\n");
        }
        va_end(args);
        sendLoggerLineMessage(buf);
    }

    void addDebugLog( char const* pszFormat, ... )
    {
        initDebugLog();
        oslDateTime currentDate;
        TimeValue   currentTime;
        osl_getSystemTime(&currentTime);
        osl_getDateTimeFromTimeValue( &currentTime, &currentDate );

        osl::MutexGuard aGuard( aMutex );

        va_list args;
        va_start(args, pszFormat);
        char buf[1024];
        int n1 = snprintf( buf, sizeof buf, "%02d:%02d:%02d pid:%u/%" SAL_PRIuUINT32 ": \"",
                           currentDate.Hours, currentDate.Minutes, currentDate.Seconds,
                           osl_getpidDebugLog(), osl::Thread::getCurrentIdentifier());
        OSL_ASSERT(n1 >= 0 && (static_cast< unsigned int >(n1) < sizeof buf - RTL_CONSTASCII_LENGTH("\"...\n")));
        int n2 = sizeof buf  - n1 - RTL_CONSTASCII_LENGTH("\"...\n");
        int n3 = vsnprintf(buf + n1, n2, pszFormat, args);
        if (n3 < 0) {
            std::strcpy(buf+n1, "\"???\n");
        } else if (n3 < n2) {
            std::strcpy(buf + n1 + n3, "\"\n");
        } else {
            std::strcpy(buf + n1 + n2 - 1, "\"...\n");
        }

        va_end(args);
        sendLoggerLineMessage(buf);
    }

    static void writeLog(char const* fileName, sal_Bool writeAlways)
    {
        oslDateTime currentDate;
        TimeValue   currentTime;
        osl_getSystemTime(&currentTime);
        osl_getDateTimeFromTimeValue( &currentTime, &currentDate );
        osl::MutexGuard aGuard( aMutex );
        // prepend absolute datetime time to file name
        char buff[1024];
        snprintf( buff, sizeof buff, "%04d%02d%02d-%02d%02d%02d-%s",
                           currentDate.Year,currentDate.Month,currentDate.Day,
                           currentDate.Hours, currentDate.Minutes, currentDate.Seconds,
                           fileName );

        if (! m_xDebugLogRing.is() )
        {
            try
            {
                ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
                if ( aContext.is() )
                    m_xDebugLogRing.set( aContext.getSingleton( "com.sun.star.logging.SingleDebugLogRing" ), uno::UNO_QUERY_THROW );
            }
            catch( uno::Exception& )
            { }
        }

        if ( m_xDebugLogRing.is() )
        {
            try
            {
                rtl::OUString aFileName(buff, std::strlen(buff), RTL_TEXTENCODING_ASCII_US);
                if(writeAlways || !m_xDebugLogRing->isEmpty())
                    m_xDebugLogRing->flushLog(aFileName);
            }
            catch( uno::Exception& )
            { }
        }        
    }

 /**
 * write log to fileName
 * the file is created in <user dir>/temp
 * if a file with the same name exists, it's overwritten
 *
 * the internal logger content is unchanged
 */
    void writeDebugLog(char const* fileName)
    {
        writeLog(fileName, true);
    }

 /**
 * write log to fileName, but oly if the logger contains records
 * the file is created in <user dir>/temp
 * if a file with the same name exists, it's overwritten
 *
 * the internal logger content is unchanged
 */
    void writeDebugLogIfNotEmpty(char const* fileName)
    {
        writeLog(fileName, false);
    }

}; //namespace tools
