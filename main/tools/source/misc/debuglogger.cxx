/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "osl/file.hxx"
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
        fprintf(stdout,"%s",pszMessage);
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

    // in the directory <user private LO conf>/user/temp do a dir of the
    // file names that contains 'debug-logger.log'
    // for every one of them, check the date, if the date is older
    // then current_time - fileAgeInHours
    // remove it
    static void deleteOlderFiles(int nFileAgeInHours )
    {
        // grab the DebugLoggerFileAgeInHours from the setup.ini/setuprc file in current installation
        // to set the file age, you can add the following parameter to setup(rc or .ini), assigning it
        // the number of hours of file age, for example 96 means 4 days:
        // DebugLoggerFileAgeInHours=96
        // please maintain the capital letters

        ::rtl::OUString aSetupFileAgeInHours = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("setup") ":DebugLoggerFileAgeInHours}" ) );
        ::rtl::Bootstrap::expandMacros( aSetupFileAgeInHours );
        if(aSetupFileAgeInHours.getLength())
            nFileAgeInHours = ( aSetupFileAgeInHours.toInt32() == 0 )? nFileAgeInHours :
                aSetupFileAgeInHours.toInt32();

        int nFileAgeInSeconds = nFileAgeInHours*3600;

        ::rtl::OUString path = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}" ) );
        rtl::Bootstrap::expandMacros( path );
        path += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/user/temp/" ) );
        osl::Directory dir( path );
         // Get the current file Status
        TimeValue TargetRemoveTime = {0,0};
        osl_getSystemTime(&TargetRemoveTime);
        // compute older file max time
        TargetRemoveTime.Seconds -= nFileAgeInSeconds;
        if( dir.reset() == osl::Directory::E_None )
        {
            for(;;)
            {
                osl::DirectoryItem item;
                if( dir.getNextItem( item ) != osl::Directory::E_None )
                    break;
                osl::FileStatus status( osl_FileStatus_Mask_FileURL );
                if( item.getFileStatus( status ) == osl::File::E_None )
                {
                    ::rtl::OUString afile = status.getFileURL();
                    if( afile.endsWithAsciiL( "debug-logger.log", 16 ) )
                    {
                        osl::FileStatus statusModTime( osl_FileStatus_Mask_ModifyTime );
                        item.getFileStatus( statusModTime );
                        TimeValue ModifyTime = statusModTime.getModifyTime();
                        if( ModifyTime.Seconds < TargetRemoveTime.Seconds )
                            osl::File::remove( afile );
                    }
                }
            }
        }
    }

  void initDebugLog()
    {
      deleteOlderFiles( 7*24 );
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
    void writeDebugLogDefaultName()
    {
        writeLog("debug-logger.log", true);
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
 * write log to fileName
 * the file is created in <user dir>/temp
 * if a file with the same name exists, it's overwritten
 *
 * the internal logger content is unchanged
 */
    void writeDebugLogDefaultNameIfNotEmpty()
    {
        writeLog("debug-logger.log", false);
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
