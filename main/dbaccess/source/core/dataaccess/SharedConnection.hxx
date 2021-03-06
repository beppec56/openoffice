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


#ifndef DBA_CORE_SHARED_CONNECTION_HXX
#define DBA_CORE_SHARED_CONNECTION_HXX

#ifndef _CONNECTIVITY_CONNECTIONWRAPPER_HXX_
#include "connectivity/ConnectionWrapper.hxx"
#endif
#ifndef _CPPUHELPER_COMPONENT_HXX_ 
#include <cppuhelper/component.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_ 
#include <connectivity/CommonTools.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XWARNINGSSUPPLIER_HPP_
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
#include <com/sun/star/sdbc/SQLWarning.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMMANDPREPARATION_HPP_
#include <com/sun/star/sdb/XCommandPreparation.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XVIEWSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_ 
#include <comphelper/sequence.hxx>
#endif


namespace dbaccess
{
	//=======================================================================================
	//= OSharedConnection: This class implements a simple forwarding of connection calls.
	//= All methods will be forwarded with exception of the set methods, which are not allowed
	//= to be called on shared connections. Instances of this class will be created when the
	//= datasource is asked for not isolated connection.
	//=======================================================================================
	typedef ::cppu::WeakComponentImplHelper1< ::com::sun::star::sdbc::XConnection
											> OSharedConnection_BASE;
	typedef ::connectivity::OConnectionWrapper	OSharedConnection_BASE2;

	class OSharedConnection :	public ::comphelper::OBaseMutex
							  , public OSharedConnection_BASE
							  , public OSharedConnection_BASE2
	{
	protected:
		virtual void SAL_CALL disposing(void);
		virtual ~OSharedConnection();
	public:
		OSharedConnection(::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >& _rxProxyConnection);

		virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException);

		virtual void SAL_CALL acquire() throw() { OSharedConnection_BASE::acquire(); }
		virtual void SAL_CALL release() throw() { OSharedConnection_BASE::release(); }
		virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException)
		{
			return ::comphelper::concatSequences( 
				OSharedConnection_BASE::getTypes(), 
				OSharedConnection_BASE2::getTypes()
			); 
		}

		virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException)
		{
			::com::sun::star::uno::Any aReturn = OSharedConnection_BASE::queryInterface(_rType);
			if ( !aReturn.hasValue() )
				aReturn = OSharedConnection_BASE2::queryInterface(_rType);
			return aReturn;
		}
		// --------------------------------------------------------------------------------
		// XCloseable
		virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
		{
			{
				::osl::MutexGuard aGuard( m_aMutex );
				::connectivity::checkDisposed(rBHelper.bDisposed);
					
			}
			dispose();
		}

		// XConnection
		virtual void SAL_CALL setAutoCommit( sal_Bool /*autoCommit*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
		{				
			throw ::com::sun::star::sdbc::SQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("This call is not allowed when sharing connections.")),*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S10000")),0,::com::sun::star::uno::Any());
		}
		virtual void SAL_CALL setReadOnly( sal_Bool /*readOnly*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
		{
			throw ::com::sun::star::sdbc::SQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("This call is not allowed when sharing connections.")),*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S10000")),0,::com::sun::star::uno::Any());
		}
		virtual void SAL_CALL setCatalog( const ::rtl::OUString& /*catalog*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
		{
			throw ::com::sun::star::sdbc::SQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("This call is not allowed when sharing connections.")),*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S10000")),0,::com::sun::star::uno::Any());
		}
		virtual void SAL_CALL setTransactionIsolation( sal_Int32 /*level*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
		{
			throw ::com::sun::star::sdbc::SQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("This call is not allowed when sharing connections.")),*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S10000")),0,::com::sun::star::uno::Any());
		}
		virtual void SAL_CALL setTypeMap( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
		{
			throw ::com::sun::star::sdbc::SQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("This call is not allowed when sharing connections.")),*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S10000")),0,::com::sun::star::uno::Any());
		}
		// XConnection
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement > SAL_CALL createStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareCall( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL nativeSQL( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL getAutoCommit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL commit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL rollback(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isClosed(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isReadOnly(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getCatalog(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTypeMap(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
	};
	
#ifdef IMPLEMENT_GET_IMPLEMENTATION_ID
	IMPLEMENT_GET_IMPLEMENTATION_ID( OSharedConnection );
#endif
//........................................................................
}	// namespace dbaccess
//........................................................................
#endif // DBA_CORE_SHARED_CONNECTION_HXX


