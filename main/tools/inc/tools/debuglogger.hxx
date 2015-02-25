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



#ifndef INCLUDED_TOOLS_GETPROCESSWORKINGDIR_HXX
#define INCLUDED_TOOLS_GETPROCESSWORKINGDIR_HXX


#include "sal/config.h"
#include <rtl/ustring.hxx>

#include "osl/diagnose.h"

#include "tools/toolsdllapi.h"
#include <cppuhelper/exc_hlp.hxx>
#include <osl/thread.h>
#include <typeinfo>
#include <boost/current_function.hpp>

namespace tools {

    TOOLS_DLLPUBLIC void addExceptionErrorDebugLog(const sal_Char* pszFileName, sal_Int32 nLine, const sal_Char* pszMessage);
    TOOLS_DLLPUBLIC void addDebugLogExc(char const* funcName,  const int lineNumber, char const* pszFormat, ... );
    TOOLS_DLLPUBLIC void addDebugLog(char const* pszFormat, ... );
    TOOLS_DLLPUBLIC void addDebugLogFunction(char const* funcName,  const int lineNumber, char const* pszFormat, ... );
    TOOLS_DLLPUBLIC void flushDebugLog(char const* fileName);
}

//macros to be used
#define DBGLOG_UNHANDLED_EXCEPTION()            \
     { \
        ::com::sun::star::uno::Any caught2( ::cppu::getCaughtException() ); \
         ::rtl::OString sMessage2( "\ncaught an exception in function:" ); \
        sMessage2 += BOOST_CURRENT_FUNCTION; \
        sMessage2 += "\ntype: "; \
        sMessage2 += ::rtl::OString( caught2.getValueTypeName().getStr(), caught2.getValueTypeName().getLength(), osl_getThreadTextEncoding() ); \
        ::com::sun::star::uno::Exception exception2; \
        caught2 >>= exception2; \
        if ( exception2.Message.getLength() ) \
        { \
            sMessage2 += "\nmessage: "; \
            sMessage2 += ::rtl::OString( exception2.Message.getStr(), exception2.Message.getLength(), osl_getThreadTextEncoding() ); \
        } \
        if ( exception2.Context.is() ) \
        { \
            const char* pContext = typeid( *exception2.Context.get() ).name(); \
            sMessage2 += "\ncontext: "; \
            sMessage2 += pContext; \
        } \
        sMessage2 += "\n"; \
        ::tools::addExceptionErrorDebugLog(OSL_THIS_FILE, __LINE__, sMessage2.getStr()); \
     } \

// a trick to substitute the standard macro declared in tools/diagnose_ex.h when compiling in no-debug mode
#if OSL_DEBUG_LEVEL == 0
#undef DBG_UNHANDLED_EXCEPTION
#define DBG_UNHANDLED_EXCEPTION() DBGLOG_UNHANDLED_EXCEPTION()
#endif

#define DBGLOG_EXCEPTION_BRIEF()            \
     { \
        ::com::sun::star::uno::Any caught2( ::cppu::getCaughtException() ); \
         ::rtl::OString sMessage2( "\ncaught an exception in function:" ); \
        sMessage2 += BOOST_CURRENT_FUNCTION; \
        sMessage2 += "\ntype: "; \
        sMessage2 += ::rtl::OString( caught2.getValueTypeName().getStr(), caught2.getValueTypeName().getLength(), osl_getThreadTextEncoding() ); \
        ::com::sun::star::uno::Exception exception2; \
        caught2 >>= exception2; \
        if ( exception2.Message.getLength() ) \
        { \
            sMessage2 += "\nmessage: "; \
            sMessage2 += ::rtl::OString( exception2.Message.getStr(), exception2.Message.getLength(), osl_getThreadTextEncoding() ); \
        } \
        if ( exception2.Context.is() ) \
        { \
            const char* pContext = typeid( *exception2.Context.get() ).name(); \
            sMessage2 += "\ncontext: "; \
            sMessage2 += pContext; \
        } \
        sMessage2 += "\n"; \
        ::tools::addDebugLogExc(OSL_THIS_FILE, __LINE__, sMessage2.getStr()); \
     } \

#define DBGLOG_STACK_SNAPSHOT(condition) \
        if(condition) \
        { \
            try \
            { \
             throw com::sun::star::uno::RuntimeException( \
                        rtl::OUString::createFromAscii( "Force stack snapshot" ), \
                        com::sun::star::uno::Reference< com::sun::star::uno::XInterface >());    \
            } \
            catch (com::sun::star::uno::RuntimeException& e) \
            { \
                DBG_UNHANDLED_EXCEPTION(); \
            } \
        } \

#define DBGLOG_TRACE    ::tools::addDebugLog

#define DBGLOG_TRACE_FUNCTION    ::tools::addDebugLogFunction

#define DBGLOG_FLUSH    ::tools::flushDebugLog

#endif
