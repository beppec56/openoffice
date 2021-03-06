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
#include "precompiled_ucb.hxx"

#include <cacheddynamicresultset.hxx>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <cachedcontentresultset.hxx>
#include <osl/diagnose.h>

using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace rtl;

CachedDynamicResultSet::CachedDynamicResultSet(
		Reference< XDynamicResultSet > xOrigin
		, const Reference< XContentIdentifierMapping > & xContentMapping
		, const Reference< XMultiServiceFactory > & xSMgr )
		: DynamicResultSetWrapper( xOrigin, xSMgr )
		, m_xContentIdentifierMapping( xContentMapping )
{
	impl_init();
}

CachedDynamicResultSet::~CachedDynamicResultSet()
{
	impl_deinit();
}

//virtual
void SAL_CALL CachedDynamicResultSet
	::impl_InitResultSetOne( const Reference< XResultSet >& xResultSet )
{
	DynamicResultSetWrapper::impl_InitResultSetOne( xResultSet );
	OSL_ENSURE( m_xSourceResultOne.is(), "need source resultset" );

	Reference< XResultSet > xCache(
        new CachedContentResultSet( m_xSMgr, m_xSourceResultOne, m_xContentIdentifierMapping ) );

	osl::Guard< osl::Mutex > aGuard( m_aMutex );
	m_xMyResultOne = xCache;
}

//virtual
void SAL_CALL CachedDynamicResultSet
	::impl_InitResultSetTwo( const Reference< XResultSet >& xResultSet )
{
	DynamicResultSetWrapper::impl_InitResultSetTwo( xResultSet );
	OSL_ENSURE( m_xSourceResultTwo.is(), "need source resultset" );

	Reference< XResultSet > xCache(
        new CachedContentResultSet( m_xSMgr, m_xSourceResultTwo, m_xContentIdentifierMapping ) );

	osl::Guard< osl::Mutex > aGuard( m_aMutex );
	m_xMyResultTwo = xCache;
}

//--------------------------------------------------------------------------
// XInterface methods.
//--------------------------------------------------------------------------
XINTERFACE_COMMON_IMPL( CachedDynamicResultSet )

Any SAL_CALL CachedDynamicResultSet
	::queryInterface( const Type&  rType )
	throw ( RuntimeException )
{
	//list all interfaces inclusive baseclasses of interfaces

	Any aRet = DynamicResultSetWrapper::queryInterface( rType );
	if( aRet.hasValue() )
		return aRet;

	aRet = cppu::queryInterface( rType,
				static_cast< XTypeProvider* >( this )
				, static_cast< XServiceInfo* >( this )
				);
	return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

//--------------------------------------------------------------------------
// XTypeProvider methods.
//--------------------------------------------------------------------------
//list all interfaces exclusive baseclasses
XTYPEPROVIDER_IMPL_4( CachedDynamicResultSet
					, XTypeProvider
					, XServiceInfo
					, XDynamicResultSet
					, XSourceInitialization
					);

//--------------------------------------------------------------------------
// XServiceInfo methods.
//--------------------------------------------------------------------------

XSERVICEINFO_NOFACTORY_IMPL_1( CachedDynamicResultSet,
				 		   OUString::createFromAscii(
							"com.sun.star.comp.ucb.CachedDynamicResultSet" ),
				 		   OUString::createFromAscii(
							CACHED_DRS_SERVICE_NAME ) );

//--------------------------------------------------------------------------
// own methds. ( inherited )
//--------------------------------------------------------------------------
//virtual
void SAL_CALL CachedDynamicResultSet
	::impl_disposing( const EventObject& Source )
	throw( RuntimeException )
{
	DynamicResultSetWrapper::impl_disposing( Source );
	m_xContentIdentifierMapping.clear();
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// class CachedDynamicResultSetFactory
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

CachedDynamicResultSetFactory::CachedDynamicResultSetFactory(
		const Reference< XMultiServiceFactory > & rSMgr )
{
	m_xSMgr = rSMgr;
}

CachedDynamicResultSetFactory::~CachedDynamicResultSetFactory()
{
}

//--------------------------------------------------------------------------
// CachedDynamicResultSetFactory XInterface methods.
//--------------------------------------------------------------------------

XINTERFACE_IMPL_3( CachedDynamicResultSetFactory,
				   XTypeProvider,
				   XServiceInfo,
				   XCachedDynamicResultSetFactory );

//--------------------------------------------------------------------------
// CachedDynamicResultSetFactory XTypeProvider methods.
//--------------------------------------------------------------------------

XTYPEPROVIDER_IMPL_3( CachedDynamicResultSetFactory,
					  XTypeProvider,
				   	  XServiceInfo,
					  XCachedDynamicResultSetFactory );

//--------------------------------------------------------------------------
// CachedDynamicResultSetFactory XServiceInfo methods.
//--------------------------------------------------------------------------

XSERVICEINFO_IMPL_1( CachedDynamicResultSetFactory,
	 		   		 OUString::createFromAscii(
					 	"com.sun.star.comp.ucb.CachedDynamicResultSetFactory" ),
	 		   		 OUString::createFromAscii(
					 	CACHED_DRS_FACTORY_NAME ) );

//--------------------------------------------------------------------------
// Service factory implementation.
//--------------------------------------------------------------------------

ONE_INSTANCE_SERVICE_FACTORY_IMPL( CachedDynamicResultSetFactory );

//--------------------------------------------------------------------------
// CachedDynamicResultSetFactory XCachedDynamicResultSetFactory methods.
//--------------------------------------------------------------------------

//virtual
Reference< XDynamicResultSet > SAL_CALL CachedDynamicResultSetFactory
	::createCachedDynamicResultSet(
		  const Reference< XDynamicResultSet > & SourceStub
		, const Reference< XContentIdentifierMapping > & ContentIdentifierMapping )
		throw( RuntimeException )
{
	Reference< XDynamicResultSet > xRet;
	xRet = new CachedDynamicResultSet( SourceStub, ContentIdentifierMapping, m_xSMgr );
	return xRet;
}


