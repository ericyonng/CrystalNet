/*!
 *  MIT License
 *  
 *  Copyright (c) 2020 ericyonng<120453674@qq.com>
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 * 
 * Date: 2023-06-11 22:56:10
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_SQL_BUILDER_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_SQL_BUILDER_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>

KERNEL_BEGIN

class SqlBuilderType
{
public:
    enum ENUMS
    {
        SELECT = 0,
        INSERT,
        REPLACE_INTO,
        UPDATE,
        DELETE_RECORD,
        CREATE_TABLE,
        TRUNCATE_TABLE,
        DROP_TABLE,
        CREATE_DB,
        DROP_DB,
    }
};

template<SqlBuilderType::ENUMS T>
class SqlBuilder
{
public:
    LibString ToSql() const
    {
        return "";
    }
};

// select field,field from table1,table2 where condition;
template<>
class SqlBuilder<SqlBuilderType::SELECT>
{
public:
    SqlBuilder(){}
    ~SqlBuilder(){}

    void Clear()
    {
        _fields.clear();
        _tables.clear();
        _where.clear();
        _orders.clear();
        _limit = -1;
    }

    SqlBuilder<SqlBuilderType::SELECT> &WithFields(const std::vector<LibString> &fields)
    {
        for(auto &v : fields)
            _fields.push_back(v);

        return *this;
    }

    SqlBuilder<SqlBuilderType::SELECT> &WithField(const LibString &field)
    {
        _fields.push_back(field);
        return *this;
    }

    SqlBuilder<SqlBuilderType::SELECT> &From(const LibString &table)
    {
        _tables.push_back(table);
        return *this;
    }

    SqlBuilder<SqlBuilderType::SELECT> &From(const std::vector<LibString> &tables)
    {
        for(auto &v : tables)
            _tables.push_back(v);

        return *this;
    }

    SqlBuilder<SqlBuilderType::SELECT> &Where(const LibString &condition)
    {
        _where = condition;
        return *this;
    }

    SqlBuilder<SqlBuilderType::SELECT> &Limit(Int32 number)
    {
        _limit = number;
        return *this;
    }

    SqlBuilder<SqlBuilderType::SELECT> &OrderBy(const LibString &content)
    {
        _orders.push_back(content);
        return *this;
    }

    SqlBuilder<SqlBuilderType::SELECT> &OrderBy(const std::vector<LibString> &contents)
    {
        for(auto &v : contents)
            _orders.push_back(v);

        return *this;
    }

    LibString ToSql() const
    {
        if(_tables.empty())
            return LibString();

        LibString sql;
        sql.AppendFormat("SELECT ");

        // fields
        if(!_fields.empty())
        {
            const Int32 count = static_cast<Int32>(_fields.size());
            for(Int32 idx = 0; idx < count; ++idx)
            {
                sql.AppendData(_fields[idx].c_str(), static_cast<Int64>(_fields[idx].size()));

                if((idx + 1) != count)
                    sql.AppendFormat(", ");
            }
        }
        else
        {
            sql.AppendFormat("* ");
        }

        {// from
            const Int32 count = static_cast<Int32>(_tables.size());
            for(Int32 idx = 0; idx < count; ++idx)
            {
                sql.AppendData(_tables[idx].c_str(), static_cast<Int64>(_tables[idx].size()));

                if((idx + 1) != count)
                {
                    sql.AppendFormat(", ");
                }
            }
        }

        {// where
            if(!_where.empty())
                sql.AppendData(_where.c_str(), static_cast<Int64>(_where.size()));
        }

        {// order by
            if(!_orders.empty())
            {
                const Int32 count = static_cast<Int32>(_orders.size());
                for(Int32 idx = 0; idx < count; ++idx)
                {
                    sql.AppendData(_orders[idx].c_str(), static_cast<Int64>(_orders[idx].size()));

                    if((idx + 1) != count)
                    {
                        sql.AppendFormat(", ");
                    }
                }
            }
        }

        // limit
        if(_limit > 0)
        {
            sql.AppendFormat("LIMIT %d", _limit);
        }

        return sql;
    }

private:
    std::vector<LibString> _fields;
    std::vector<LibString> _tables;
    LibString _where;
    std::vector<LibString> _orders;
    Int32 _limit = -1;
};

// insert field,field into table1 values(xx,xx,...);
template<>
class SqlBuilder<SqlBuilderType::INSERT>
{
public:
    SqlBuilder(){}
    ~SqlBuilder(){}

    void Clear()
    {
        _fields.clear();
        _table.clear();
        _values.clear();
        _valuesFromSql.clear();
    }

    SqlBuilder<SqlBuilderType::INSERT> &Fields(const std::vector<LibString> &fields)
    {
        _fields = fields;
        return *this;
    }

    SqlBuilder<SqlBuilderType::INSERT> &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    SqlBuilder<SqlBuilderType::INSERT> &Values(const std::vector<LibString> &values)
    {
        _valuesFromSql.clear();
        _values = values;

        return *this;
    }

    SqlBuilder<SqlBuilderType::INSERT> &ValuesFrom(const SqlBuilder<SqlBuilderType::SELECT> &src)
    {
        const auto &sql = src.ToSql();
        if(sql.empty())
            return *this;

        _valuesFromSql = sql;
        _values.clear();
        _fields.clear();

        return *this;
    }

    LibString ToSql() const
    {
        if(UNLIKELY(_table.empty()))
            return "";

        if(UNLIKELY(_values.empty() && _valuesFromSql.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("INSERT INTO `%s`", _table.c_str());
        if(!_fields.empty())
        {
            sql.AppendFormat("(");
            const Int32 count = static_cast<Int32>(_fields.size());
            for(Int32 idx = 0; idx < count; ++idx)
            {
                sql.AppendFormat("`");
                sql.AppendData(_fields[idx].c_str(), static_cast<Int64>(_fields[idx].size()));
                sql.AppendFormat("`");
                if((idx + 1) != count)
                    sql.AppendFormat(", ");
            }
            sql.AppendFormat(")");
        }

        if(!_values.empty())
        {
            sql.AppendFormat(" VALUE(");
            const Int32 count = static_cast<Int32>(_values.size());
            for(Int32 idx = 0; idx < count; ++idx)
            {
                sql.AppendData(_values[idx].c_str(), static_cast<Int64>(_values[idx].size()));
                if((idx + 1) != count)
                    sql.AppendFormat(",");
            }
            sql.AppendFormat(")");
            return sql;
        }

        sql.AppendFormat(" ");
        sql.AppendData(_valuesFromSql.c_str(), static_cast<Int64>(_valuesFromSql.size()));

        return sql;
    }

private:
    std::vector<LibString> _fields;
    LibString _table;
    std::vector<LibString> _values;
    LibString _valuesFromSql;
};

// replace into t(field,field) into table1 values(xx,xx,...);
template<>
class SqlBuilder<SqlBuilderType::REPLACE_INTO>
{
public:
    SqlBuilder(){}
    ~SqlBuilder(){}


    void Clear()
    {
        _fields.clear();
        _table.clear();
        _values.clear();
        _valuesFromSql.clear();
    }

    SqlBuilder<SqlBuilderType::REPLACE_INTO> &Fields(const std::vector<LibString> &fields)
    {
        _fields = fields;
        return *this;
    }

    SqlBuilder<SqlBuilderType::REPLACE_INTO> &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    SqlBuilder<SqlBuilderType::REPLACE_INTO> &Values(const std::vector<LibString> &values)
    {
        _valuesFromSql.clear();
        _values = values;

        return *this;
    }

    SqlBuilder<SqlBuilderType::REPLACE_INTO> &ValuesFrom(const SqlBuilder<SqlBuilderType::SELECT> &src)
    {
        const auto &sql = src.ToSql();
        if(sql.empty())
            return *this;

        _valuesFromSql = sql;
        _values.clear();
        _fields.clear();

        return *this;
    }

    LibString ToSql() const
    {
        if(UNLIKELY(_table.empty()))
            return "";

        if(UNLIKELY(_values.empty() && _valuesFromSql.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("REPLACE INTO `%s`", _table.c_str());
        if(!_fields.empty())
        {
            sql.AppendFormat("(");
            const Int32 count = static_cast<Int32>(_fields.size());
            for(Int32 idx = 0; idx < count; ++idx)
            {
                sql.AppendFormat("`");
                sql.AppendData(_fields[idx].c_str(), static_cast<Int64>(_fields[idx].size()));
                sql.AppendFormat("`");
                if((idx + 1) != count)
                    sql.AppendFormat(", ");
            }
            sql.AppendFormat(")");
        }

        if(!_values.empty())
        {
            sql.AppendFormat(" VALUE(");
            const Int32 count = static_cast<Int32>(_values.size());
            for(Int32 idx = 0; idx < count; ++idx)
            {
                sql.AppendData(_values[idx].c_str(), static_cast<Int64>(_values[idx].size()));
                if((idx + 1) != count)
                    sql.AppendFormat(",");
            }
            sql.AppendFormat(")");
            return sql;
        }

        sql.AppendFormat(" ");
        sql.AppendData(_valuesFromSql.c_str(), static_cast<Int64>(_valuesFromSql.size()));

        return sql;
    }

private:
    std::vector<LibString> _fields;
    LibString _table;
    std::vector<LibString> _values;
    LibString _valuesFromSql;
};

// update table set field1=xxx, field2=xxx,... where xxx
template<>
class SqlBuilder<SqlBuilderType::UPDATE>
{
public:
    SqlBuilder(){}
    ~SqlBuilder(){}

    void Clear()
    {
        _kvs.clear();
        _table.clear();
        _where.clear();
    }

    SqlBuilder<SqlBuilderType::UPDATE> &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    SqlBuilder<SqlBuilderType::UPDATE> &Set(const LibString &field, const LibString &value)
    {
        LibString kv;

        // field
        kv.AppendFormat("`");
        kv.AppendData(field);
        kv.AppendFormat("`");

        // value
        kv.AppendFormat(" = ");
        kv.AppendData(value);

        _kvs.push_back(kv);

        return *this;
    }

    SqlBuilder<SqlBuilderType::UPDATE> &Where(const LibString &condition)
    {
        _where = condition;
        return *this;
    }

    LibString ToSql() const
    {
        if(UNLIKELY(_kvs.empty() || _table.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("UPDATE `%s` SET ", _table.c_str());

        const Int32 count = static_cast<Int32>(_kvs.size());
        for(Int32 idx = 0; idx < count; ++idx)
        {
            sql.AppendData(_kvs[idx]);
            
            if((idx + 1) != count)
                sql.AppendFormat(", ");
        }

        if(LIKELY(!_where.empty()))
            sql.AppendData(_where);

        return sql;
    }

private:
    std::vector<LibString> _kvs;
    LibString _table;
    LibString _where;
};

// delete from
template<>
class SqlBuilder<SqlBuilderType::DELETE_RECORD>
{
public:
    SqlBuilder() {}
    ~SqlBuilder() {}

    void Clear()
    {
        _table.clear();
        _where.clear();
    }

    SqlBuilder<SqlBuilderType::DELETE_RECORD> &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    SqlBuilder<SqlBuilderType::DELETE_RECORD> &Where(const LibString &condition)
    {
        _where = condition;
        return *this;
    }
    
    LibString ToSql() const
    {
        if(UNLIKELY(_table.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("DELETE FROM %s", _table.c_str());

        if(!_where.empty())
        {
            sql.AppendFormat(" WHERE ");
            sql.AppendData(_where);
        }

        return sql;
    }


private:
    LibString _table;
    LibString _where;
};

// create table if not exists `xxx`() xxx
template<>
class SqlBuilder<SqlBuilderType::CREATE_TABLE>
{
public:
    SqlBuilder() {}
    ~SqlBuilder() {}

    void Clear()
    {
        _table.clear();
        _fields.clear();
        _engine = "InnoDB";
        _charset = "utf8mb4";
        _collate = "utf8mb4_bin";
        _rowFormat = "DYNAMIC";
        _comment.clear();
    }

    SqlBuilder<SqlBuilderType::CREATE_TABLE> &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    SqlBuilder<SqlBuilderType::CREATE_TABLE> &Field(const LibString &field)
    {
        _fields.push_back(field);
        return *this;
    }
    
    SqlBuilder<SqlBuilderType::CREATE_TABLE> &Engine(const LibString &engine)
    {
        _engine = engine;
        return *this;
    }

    SqlBuilder<SqlBuilderType::CREATE_TABLE> &Charset(const LibString &charset)
    {
        _charset = charset;
        return *this;
    }

    SqlBuilder<SqlBuilderType::CREATE_TABLE> &Collate(const LibString &collate)
    {
        _collate = collate;
        return *this;
    }

    LibString ToSql() const
    {
        if(UNLIKELY(_table.empty() || _fields.empty() || _engine.empty() || _charset.empty() || _collate.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("CREATE TABLE IF NOT EXISTS `%s`(", _table.c_str());

        const Int32 count = static_cast<Int32>(_fields.size());
        for(Int32 idx = 0; idx < count; ++idx)
        {
            sql.AppendData(_fields[idx]);
            
            if((idx + 1) != count)
                sql.AppendFormat(",");
        }

        sql.AppendFormat(") ");

        sql.AppendFormat("ENGINE=%s DEFAULT CHARSET=%s, COLLATE = %s", _engine.c_str(), _charset.c_str(), _collate.c_str());

        if(!_rowFormat.empty())
            sql.AppendFormat(", ROW_FORMAT = %s", _rowFormat.c_str());

        if(!_comment.empty())
            sql.AppendFormat(", COMMENT = \"%s\"", _comment.c_str());

        return sql;
    }

private:
    LibString _table;
    std::vector<LibString> _fields;
    LibString _engine = "InnoDB";
    LibString _charset = "utf8mb4";
    LibString _collate = "utf8mb4_bin";
    LibString _rowFormat = "DYNAMIC";
    LibString _comment;
};

// truncate table `xxx`
template<>
class SqlBuilder<SqlBuilderType::TRUNCATE_TABLE>
{
public:
    SqlBuilder() {}
    ~SqlBuilder() {}

    void Clear()
    {
        _table.clear();
    }

    SqlBuilder<SqlBuilderType::TRUNCATE_TABLE> &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    LibString ToSql() const
    {
        if(UNLIKELY(_table.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("TRUNCATE TABLE `%s`", _table.c_str());
        return sql;
    }


private:
    LibString _table;
};

// drop table if exists `xxx`
template<>
class SqlBuilder<SqlBuilderType::DROP_TABLE>
{
public:
    SqlBuilder() {}
    ~SqlBuilder() {}

    void Clear()
    {
        _table.clear();
    }

    SqlBuilder<SqlBuilderType::DROP_TABLE> &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    LibString ToSql() const
    {
        if(UNLIKELY(_table.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("DROP TABLE IF EXISTS `%s`", _table.c_str());
        return sql;
    }

private:
    LibString _table;
};

// create DATABASE if not exists `xxx`
template<>
class SqlBuilder<SqlBuilderType::CREATE_DB>
{
public:
    SqlBuilder() {}
    ~SqlBuilder() {}

    void Clear()
    {
        _db.clear();
        _charset = "utf8mb4";
        _collate = "utf8mb4_bin";
    }

    SqlBuilder<SqlBuilderType::CREATE_DB> &DB(const LibString &db)
    {
        _db = db;
        return *this;
    }

    SqlBuilder<SqlBuilderType::CREATE_DB> &Charset(const LibString &charset)
    {
        _charset = charset;
        return *this;
    }

    SqlBuilder<SqlBuilderType::CREATE_DB> &Collate(const LibString &collate)
    {
        _collate = collate;
        return *this;
    }

    LibString ToSql() const
    {
        if(UNLIKELY(_db.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("CREATE DATABASE IF NOT EXISTS `%s`", _db.c_str());

        if(!_charset.empty())
            sql.AppendFormat(" DEFAULT CHARACTER SET %s", _charset.c_str());

        if(!_collate.empty())
            sql.AppendFormat(" DEFAULT COLLATE %s", _collate.c_str());

        return sql;
    }

private:
    LibString _db;
    LibString _charset = "utf8mb4";
    LibString _collate = "utf8mb4_bin";
};

// create DATABASE if not exists `xxx`
template<>
class SqlBuilder<SqlBuilderType::DROP_DB>
{
public:
    SqlBuilder() {}
    ~SqlBuilder() {}

    SqlBuilder<SqlBuilderType::DROP_DB> &DB(const LibString &db)
    {
        _db = db;
        return *this;
    }

    LibString ToSql() const
    {
        if(UNLIKELY(_db.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("DROP DATABASE IF EXISTS `%s`", _db.c_str());

        return sql;
    }
    
private:
    LibString _db;
};

KERNEL_END

#endif

