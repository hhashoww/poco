//
// ODBCColumn.cpp
//
// $Id: //poco/Main/Data/ODBC/src/ODBCColumn.cpp#4 $
//
// Library: ODBC
// Package: ODBC
// Module:  ODBCColumn
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Poco/Data/ODBC/ODBCColumn.h"
#include "Poco/Data/ODBC/Utility.h"


namespace Poco {
namespace Data {
namespace ODBC {


ODBCColumn::ODBCColumn(const StatementHandle& rStmt, std::size_t position) : 
	MetaColumn(position),
	_rStmt(rStmt)
{
	init();
}

	
ODBCColumn::~ODBCColumn()
{
}


void ODBCColumn::getDescription()
{
	memset(_columnDesc.name, 0, NAME_BUFFER_LENGTH);
	_columnDesc.nameBufferLength = 0;
	_columnDesc.dataType = 0;
	_columnDesc.size = 0;
	_columnDesc.decimalDigits = 0;
	_columnDesc.isNullable = 0;

	if (Utility::isError(SQLDescribeCol(_rStmt, 
		(SQLUSMALLINT) position() + 1, // ODBC columns are 1-based
		_columnDesc.name, 
		NAME_BUFFER_LENGTH,
		&_columnDesc.nameBufferLength, 
		&_columnDesc.dataType,
		&_columnDesc.size, 
		&_columnDesc.decimalDigits, 
		&_columnDesc.isNullable)))
	{
		throw StatementException(_rStmt);
	}
}


void ODBCColumn::init()
{
	getDescription();

	if (Utility::isError(SQLColAttribute(_rStmt,
			(SQLUSMALLINT) position() + 1, // ODBC columns are 1-based
			SQL_DESC_LENGTH,
			0,
			0,
			0,
			reinterpret_cast<long int*>(&_dataLength))))
	{
		throw StatementException(_rStmt);
	}

	setName(std::string((char*) _columnDesc.name));
	setLength(_columnDesc.size);
	setPrecision(_columnDesc.decimalDigits);
	setNullable(SQL_NULLABLE == _columnDesc.isNullable);
	switch(_columnDesc.dataType)
	{
	case SQL_CHAR:
	case SQL_VARCHAR:
	case SQL_LONGVARCHAR:
		setType(MetaColumn::FDT_STRING); break;
	case SQL_TINYINT:
		setType(MetaColumn::FDT_INT8); break;
	case SQL_SMALLINT:
		setType(MetaColumn::FDT_INT16); break;
	case SQL_INTEGER:
	case SQL_DECIMAL:
		setType(MetaColumn::FDT_INT32); break;
	case SQL_BIGINT:
		setType(MetaColumn::FDT_INT64); break;
	case SQL_NUMERIC:
	case SQL_DOUBLE:
	case SQL_FLOAT:
		setType(MetaColumn::FDT_DOUBLE); break;
	case SQL_REAL:
		setType(MetaColumn::FDT_FLOAT); break;
	case SQL_BINARY:
	case SQL_VARBINARY:
	case SQL_LONGVARBINARY:
	case -98:// IBM DB2 non-standard type
		setType(MetaColumn::FDT_BLOB); break;
	default:
		throw DataFormatException("Unsupported data type.");
	}
}


} } } // namespace Poco::Data::ODBC
