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
        CREATE_TABLE,
        TRAUNCATE_TABLE,
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

KERNEL_END

#endif

