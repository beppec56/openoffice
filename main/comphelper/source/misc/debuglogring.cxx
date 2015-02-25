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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

#include <rtl/bootstrap.hxx>
#include <com/sun/star/frame/DoubleInitializationException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/io/XTruncate.hpp>

#include <comphelper_module.hxx>
#include <comphelper/processfactory.hxx>

#include "debuglogring.hxx"

using namespace ::com::sun::star;

namespace comphelper
{

// ----------------------------------------------------------
ODebugLogRing::ODebugLogRing( const uno::Reference< uno::XComponentContext >& /*xContext*/ )
: m_aMessages( DEBUGLOGRING_SIZE )
, m_bInitialized( sal_False )
, m_bFull( sal_False )
, m_nPos( 0 )
{
}

// ----------------------------------------------------------
ODebugLogRing::~ODebugLogRing()
{
}

// ----------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL ODebugLogRing::getSupportedServiceNames_static()
{
    uno::Sequence< rtl::OUString > aResult( 1 );
    aResult[0] = getServiceName_static();
    return aResult;
}

// ----------------------------------------------------------
// the component
::rtl::OUString SAL_CALL ODebugLogRing::getImplementationName_static()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.logging.DebugLogRing" ) );
}

// ----------------------------------------------------------
// the singleton
::rtl::OUString SAL_CALL ODebugLogRing::getSingletonName_static()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.logging.SingleDebugLogRing" ) );
}

// ----------------------------------------------------------
// interface (the service)
::rtl::OUString SAL_CALL ODebugLogRing::getServiceName_static()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.logging.DebugLogRing" ) );
}

// ----------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL ODebugLogRing::Create( const uno::Reference< uno::XComponentContext >& rxContext )
{
    return static_cast< cppu::OWeakObject* >( new ODebugLogRing( rxContext ) );
}

// XDebugLogRing implementation
// ----------------------------------------------------------
void SAL_CALL ODebugLogRing::logString( const ::rtl::OUString& aMessage ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    m_aMessages[m_nPos] = aMessage;
    if ( ++m_nPos >= m_aMessages.getLength() )
    {
        m_nPos = 0;
        m_bFull = sal_True;
    }

    // if used once then default initialized
    m_bInitialized = sal_True;
}

// ----------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL ODebugLogRing::getCollectedLog() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nResLen = m_bFull ? m_aMessages.getLength() : m_nPos;
    sal_Int32 nStart = m_bFull ? m_nPos : 0;
    uno::Sequence< ::rtl::OUString > aResult( nResLen );

    for ( sal_Int32 nInd = 0; nInd < nResLen; nInd++ )
        aResult[nInd] = m_aMessages[ ( nStart + nInd ) % m_aMessages.getLength() ];

    // if used once then default initialized
    m_bInitialized = sal_True;

    return aResult;
}

namespace
{
    void lcl_writeStringInStream( const uno::Reference< io::XOutputStream >& xOutStream, const ::rtl::OUString& aString )
    {
        if ( xOutStream.is() )
        {
            ::rtl::OString aStrLog = ::rtl::OUStringToOString( aString, RTL_TEXTENCODING_UTF8 );
            uno::Sequence< sal_Int8 > aLogData( (const sal_Int8*)aStrLog.getStr(), aStrLog.getLength() );
            xOutStream->writeBytes( aLogData );
        }
    }
}

// ----------------------------------------------------------
void SAL_CALL ODebugLogRing::flushLog( const ::rtl::OUString& aFileName ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::rtl::OUString aFileURL = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}" ) );
    ::rtl::Bootstrap::expandMacros( aFileURL );
    ::rtl::OUString aBuildID = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("setup") ":buildid}" ) );
    ::rtl::Bootstrap::expandMacros( aBuildID );

    if ( aFileURL.getLength() )
    {
        aFileURL += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/user/temp/" ) );
        aFileURL += aFileName;
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory(), uno::UNO_SET_THROW );
            uno::Reference< ucb::XSimpleFileAccess > xSimpleFileAccess( xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.SimpleFileAccess" ) ) ), uno::UNO_QUERY_THROW );
            uno::Reference< io::XStream > xStream( xSimpleFileAccess->openFileReadWrite( aFileURL ), uno::UNO_SET_THROW );
            uno::Reference< io::XOutputStream > xOutStream( xStream->getOutputStream(), uno::UNO_SET_THROW );
            uno::Reference< io::XTruncate > xTruncate( xOutStream, uno::UNO_QUERY_THROW );
            xTruncate->truncate();

            if ( aBuildID.getLength() )
            {
                aBuildID += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "\n" ) );
                lcl_writeStringInStream( xOutStream, aBuildID );
            }

            sal_Int32 nResLen = m_bFull ? m_aMessages.getLength() : m_nPos;
            sal_Int32 nStart = m_bFull ? m_nPos : 0;
            uno::Sequence< ::rtl::OUString > aResult( nResLen );

            for ( sal_Int32 nInd = 0; nInd < nResLen; nInd++ )
                lcl_writeStringInStream( xOutStream, m_aMessages[ ( nStart + nInd ) % m_aMessages.getLength() ] );
            xOutStream->flush();
        }
        catch( uno::Exception& )
        {}
    }
}

// XInitialization
// ----------------------------------------------------------
void SAL_CALL ODebugLogRing::initialize( const uno::Sequence< uno::Any >& aArguments ) throw (uno::Exception, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bInitialized )
        throw frame::DoubleInitializationException();

    if ( !m_refCount )
        throw uno::RuntimeException(); // the object must be refcounted already!

    sal_Int32 nLen = 0;
    if ( aArguments.getLength() == 1 && ( aArguments[0] >>= nLen ) && nLen )
        m_aMessages.realloc( nLen );
    else
        throw lang::IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Nonnull size is expected as the first argument!" ) ),
                uno::Reference< uno::XInterface >(),
                0 );

    m_bInitialized = sal_True;
}

// XServiceInfo
// ----------------------------------------------------------
::rtl::OUString SAL_CALL ODebugLogRing::getImplementationName() throw (uno::RuntimeException)
{
    return getImplementationName_static();
}

// ----------------------------------------------------------
::sal_Bool SAL_CALL ODebugLogRing::supportsService( const ::rtl::OUString& aServiceName ) throw (uno::RuntimeException)
{
    const uno::Sequence< rtl::OUString > & aSupportedNames = getSupportedServiceNames_static();
    for ( sal_Int32 nInd = 0; nInd < aSupportedNames.getLength(); nInd++ )
    {
        if ( aSupportedNames[ nInd ].equals( aServiceName ) )
            return sal_True;
    }

    return sal_False;
}

// ----------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL ODebugLogRing::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return getSupportedServiceNames_static();
}

} // namespace comphelper

void createRegistryInfo_DebugLogRing()
{
    static ::comphelper::module::OAutoRegistration< ::comphelper::ODebugLogRing > aAutoRegistration;
    static ::comphelper::module::OSingletonRegistration< ::comphelper::ODebugLogRing > aSingletonRegistration;
}
