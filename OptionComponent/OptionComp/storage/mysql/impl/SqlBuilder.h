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

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/StringUtil.h>

KERNEL_BEGIN

class SqlBuilderType
{
public:
    enum ENUMS
    {
        SELECT = 0,
        INSERT,

        // 如果数据不存在则直接insert, 如果存在则先删除再insert, 前者affected_row是1后者affected_row是2
        REPLACE_INTO,

        UPDATE,
        DELETE_RECORD,
        CREATE_TABLE,

        // 删除表数据,并把自增id归0
        TRUNCATE_TABLE,
        DROP_TABLE,

        CREATE_DB,
        DROP_DB,

        // 表结构
        ALTER_TABLE,
        SHOW_INDEX,

        // 优化表会对表空间进行清理,比如delete的数据进行真正的删除
        OPTIMIZE_TABLE,

        START_TRANSACTION,      // 开启事务
        SET_AUTOCOMMIT,         // 设置自动提交标志
        ROLLBACK,               // 数据回滚操作, MyISAM引擎不支持回滚
        COMMIT_TRANSACTION,     // 提交事务
    };
};

// 在全文索引时候指定的解析器
class FullTextParser
{
public:
    static const LibString WITH_PARSER_NGRAM;
};

class SqlEscape
{
public:
    static void escape(KERNEL_NS::LibString &str);
};

class SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT(SqlBuilder);

public:
    SqlBuilder(Int32 type)
    :_type(type)
    {

    }
    
    virtual ~SqlBuilder()
    {

    }

    virtual LibString ToSql() const
    {
        return "";
    }

    virtual LibString Dump() const = 0;

    virtual LibString ToString() const
    {
        return LibString().AppendFormat("sql builder type:%d", _type);
    }

    virtual void Release() = 0;

    Int32 _type;
};

// select field,field from table1,table2 where condition;
class SelectSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, SelectSqlBuilder);

public:
    SelectSqlBuilder()
    :SqlBuilder(SqlBuilderType::SELECT)
    {

    }

    ~SelectSqlBuilder()
    {

    }

    virtual void Release() override
    {
        SelectSqlBuilder::DeleteThreadLocal_SelectSqlBuilder(this);
    }

    SelectSqlBuilder &Clear()
    {
        _fields.clear();
        _tables.clear();
        _where.clear();
        _orders.clear();
        _groupBy.clear();
        _limit = -1;
        _db.clear();
        
        return *this;
    }

    SelectSqlBuilder &WithFields(const std::vector<LibString> &fields)
    {
        for(auto &v : fields)
            _fields.push_back(v);

        return *this;
    }

    SelectSqlBuilder &WithField(const LibString &field)
    {
        _fields.push_back(field);
        return *this;
    }

    SelectSqlBuilder &From(const LibString &table)
    {
        _tables.push_back(table);
        return *this;
    }

    SelectSqlBuilder &From(const std::vector<LibString> &tables)
    {
        for(auto &v : tables)
            _tables.push_back(v);

        return *this;
    }

    SelectSqlBuilder &DB(const LibString &db)
    {
        _db = db;
        return *this;
    }

    SelectSqlBuilder &Where(const LibString &condition)
    {
        _where = condition;
        return *this;
    }

    SelectSqlBuilder &Limit(Int32 number)
    {
        _limit = number;
        return *this;
    }

    SelectSqlBuilder &OrderBy(const LibString &content)
    {
        _orders.push_back(content);
        return *this;
    }

    SelectSqlBuilder &OrderBy(const std::vector<LibString> &contents)
    {
        for(auto &v : contents)
            _orders.push_back(v);

        return *this;
    }

    SelectSqlBuilder &GroupBy(const LibString &content)
    {
        _groupBy.push_back(content);
        return *this;
    }

    SelectSqlBuilder &GroupBy(const std::vector<LibString> &contents)
    {
        for(auto &v : contents)
            _groupBy.push_back(v);

        return *this;
    }

    LibString Dump() const override
    {
        if(_tables.empty())
            return LibString();
        if(_db.empty())
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
            sql.AppendFormat("*");
        }

        {// from
            sql.AppendFormat(" FROM ");
            const Int32 count = static_cast<Int32>(_tables.size());
            for(Int32 idx = 0; idx < count; ++idx)
            {
                sql.AppendData(_db);
                sql.AppendData(".");
                sql.AppendData(_tables[idx]);

                if((idx + 1) != count)
                {
                    sql.AppendFormat(", ");
                }
            }
        }

        {// where
            if(!_where.empty())
            {
                sql.AppendFormat(" WHERE ");
                auto data = _where;
                SqlEscape::escape(data);
                sql.AppendData(data);
            }
        }

        // group by 需要在where之后, order by之前
        {// group by
            if(!_groupBy.empty())
            {
                sql.AppendFormat(" GROUP BY ");
                const Int32 count = static_cast<Int32>(_groupBy.size());
                for(Int32 idx = 0; idx < count; ++idx)
                {
                    sql.AppendData(_groupBy[idx].c_str(), static_cast<Int64>(_groupBy[idx].size()));

                    if((idx + 1) != count)
                    {
                        sql.AppendFormat(", ");
                    }
                }
            }
        }

        {// order by
            if(!_orders.empty())
            {
                sql.AppendFormat(" ORDER BY ");
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
            sql.AppendFormat(" LIMIT %d", _limit);
        }

        return sql;
    }

    LibString ToSql() const override
    {
        if(_tables.empty())
            return LibString();
        if(_db.empty())
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
            sql.AppendFormat("*");
        }

        {// from
            sql.AppendFormat(" FROM ");
            const Int32 count = static_cast<Int32>(_tables.size());
            for(Int32 idx = 0; idx < count; ++idx)
            {
                sql.AppendData(_db);
                sql.AppendData(".");
                sql.AppendData(_tables[idx]);

                if((idx + 1) != count)
                {
                    sql.AppendFormat(", ");
                }
            }
        }

        {// where
            if(!_where.empty())
            {
                sql.AppendFormat(" WHERE ");
                sql.AppendData(_where);
            }
        }

        // group by 需要在where之后, order by之前
        {// group by
            if(!_groupBy.empty())
            {
                sql.AppendFormat(" GROUP BY ");
                const Int32 count = static_cast<Int32>(_groupBy.size());
                for(Int32 idx = 0; idx < count; ++idx)
                {
                    sql.AppendData(_groupBy[idx].c_str(), static_cast<Int64>(_groupBy[idx].size()));

                    if((idx + 1) != count)
                    {
                        sql.AppendFormat(", ");
                    }
                }
            }
        }

        {// order by
            if(!_orders.empty())
            {
                sql.AppendFormat(" ORDER BY ");
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
            sql.AppendFormat(" LIMIT %d", _limit);
        }

        return sql;
    }

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::SELECT fields:[");
        for(auto &data : _fields)
        {
            info.AppendData(data);
            info.AppendData(",");
        }
        info.AppendFormat("],\n");
        info.AppendFormat("tables:[");
        for(auto &data : _tables)
        {
            info.AppendData(data);
            info.AppendData(",");
        }
        info.AppendFormat("]\n");
        info.AppendFormat("db:%s\n", _db.c_str());
        info.AppendFormat("where:%s\n", _where.c_str());
        
        info.AppendFormat("groupby:[");
        for(auto &data : _groupBy)
        {
            info.AppendData(data);
            info.AppendData(",");
        }
        info.AppendFormat("]\n");

        info.AppendFormat("orders:[");
        for(auto &data : _orders)
        {
            info.AppendData(data);
            info.AppendData(",");
        }
        info.AppendFormat("]\n");
        info.AppendFormat("limit:%d", _limit);

        return info;
    }

private:
    std::vector<LibString> _fields;
    std::vector<LibString> _tables;
    LibString _db;
    LibString _where;
    std::vector<LibString> _orders;
    std::vector<LibString> _groupBy;
    Int32 _limit = -1;
};

// insert field,field into table1 values(xx,xx,...);
class InsertSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, InsertSqlBuilder);

public:
    InsertSqlBuilder()
    :SqlBuilder(SqlBuilderType::INSERT)
    {

    }
    ~InsertSqlBuilder()
    {

    }

    virtual void Release() override
    {
        InsertSqlBuilder::DeleteThreadLocal_InsertSqlBuilder(this);
    }

    InsertSqlBuilder &Clear()
    {
        _fields.clear();
        _table.clear();
        _values.clear();
        _valuesFromSql.clear();
        _db.clear();
        return *this;
    }

    InsertSqlBuilder &Fields(const std::vector<LibString> &fields)
    {
        _fields = fields;
        return *this;
    }

    InsertSqlBuilder &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    InsertSqlBuilder &DB(const LibString &db)
    {
        _db = db;
        return *this;
    }

    InsertSqlBuilder &Values(const std::vector<LibString> &values)
    {
        _valuesFromSql.clear();
        _values = values;

        return *this;
    }

    InsertSqlBuilder &ValuesFrom(const SelectSqlBuilder &src)
    {
        const auto &sql = src.ToSql();
        if(sql.empty())
            return *this;

        _valuesFromSql = sql;
        _values.clear();
        _fields.clear();

        return *this;
    }

    LibString Dump() const override
    {
        if(UNLIKELY(_table.empty()))
            return "";

        if(UNLIKELY(_db.empty()))
            return "";

        if(UNLIKELY(_values.empty() && _valuesFromSql.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("INSERT INTO %s.`%s`", _db.c_str(), _table.c_str());
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
                auto data = _values[idx];
                SqlEscape::escape(data);
                sql.AppendData(data);
                if((idx + 1) != count)
                    sql.AppendFormat(",");
            }
            sql.AppendFormat(")");
            return sql;
        }

        sql.AppendFormat(" ");
        auto fromSql = _valuesFromSql;
        SqlEscape::escape(fromSql);
        sql.AppendData(fromSql);

        return sql;
    }

    LibString ToSql() const override
    {
        if(UNLIKELY(_table.empty()))
            return "";

        if(UNLIKELY(_db.empty()))
            return "";

        if(UNLIKELY(_values.empty() && _valuesFromSql.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("INSERT INTO %s.`%s`", _db.c_str(), _table.c_str());
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

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::INSERT fields:[");
        for(auto &data : _fields)
        {
            info.AppendData(data);
            info.AppendData(",");
        }
        info.AppendFormat("],\n");
        info.AppendFormat("table:%s\n", _table.c_str());
        info.AppendFormat("db:%s\n", _db.c_str());
        info.AppendFormat("values:[");
        for(auto &data : _values)
        {
            info.AppendData(data);
            info.AppendData(",");
        }
        info.AppendFormat("],\n");

        info.AppendFormat("valuesFromSql:\n");
        info.AppendData(_valuesFromSql);
        
        return info;
    }

private:
    std::vector<LibString> _fields;
    LibString _table;
    LibString _db;
    std::vector<LibString> _values;
    LibString _valuesFromSql;
};

// replace into t(field,field) into table1 values(xx,xx,...);
class ReplaceIntoSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, ReplaceIntoSqlBuilder);

public:
    ReplaceIntoSqlBuilder()
    :SqlBuilder(SqlBuilderType::REPLACE_INTO)
    {}
    ~ReplaceIntoSqlBuilder(){}

    virtual void Release() override
    {
        ReplaceIntoSqlBuilder::DeleteThreadLocal_ReplaceIntoSqlBuilder(this);
    }

    ReplaceIntoSqlBuilder &Clear()
    {
        _fields.clear();
        _table.clear();
        _values.clear();
        _valuesFromSql.clear();
        _db.clear();
        return *this;
    }

    ReplaceIntoSqlBuilder &Fields(const std::vector<LibString> &fields)
    {
        _fields = fields;
        return *this;
    }

    ReplaceIntoSqlBuilder &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    ReplaceIntoSqlBuilder &DB(const LibString &db)
    {
        _db = db;
        return *this;
    }

    ReplaceIntoSqlBuilder &Values(const std::vector<LibString> &values)
    {
        _valuesFromSql.clear();
        _values = values;

        return *this;
    }

    ReplaceIntoSqlBuilder &ValuesFrom(const SelectSqlBuilder &src)
    {
        const auto &sql = src.ToSql();
        if(sql.empty())
            return *this;

        _valuesFromSql = sql;
        _values.clear();
        _fields.clear();

        return *this;
    }

    LibString Dump() const override
    {
       if(UNLIKELY(_table.empty()))
            return "";

        if(UNLIKELY(_db.empty()))
            return "";

        if(UNLIKELY(_values.empty() && _valuesFromSql.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("REPLACE INTO %s.`%s`", _db.c_str(), _table.c_str());
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
                auto data = _values[idx];
                SqlEscape::escape(data);
                sql.AppendData(data);
                if((idx + 1) != count)
                    sql.AppendFormat(",");
            }
            sql.AppendFormat(")");
            return sql;
        }

        sql.AppendFormat(" ");
        auto fromSql = _valuesFromSql;
        SqlEscape::escape(fromSql);
        sql.AppendData(fromSql);

        return sql;
    }

    LibString ToSql() const override
    {
        if(UNLIKELY(_table.empty()))
            return "";

        if(UNLIKELY(_db.empty()))
            return "";

        if(UNLIKELY(_values.empty() && _valuesFromSql.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("REPLACE INTO %s.`%s`", _db.c_str(), _table.c_str());
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

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::REPLACE_INTO fields:[");
        for(auto &data : _fields)
        {
            info.AppendData(data);
            info.AppendData(",");
        }
        info.AppendFormat("],\n");
        info.AppendFormat("db:%s\n", _db.c_str());
        info.AppendFormat("table:%s\n", _table.c_str());
        info.AppendFormat("values:[");
        for(auto &data : _values)
        {
            info.AppendData(data);
            info.AppendData(",");
        }
        info.AppendFormat("],\n");

        info.AppendFormat("valuesFromSql:\n");
        info.AppendData(_valuesFromSql);
        
        return info;
    }

private:
    std::vector<LibString> _fields;
    LibString _db;
    LibString _table;
    std::vector<LibString> _values;
    LibString _valuesFromSql;
};

// update table set field1=xxx, field2=xxx,... where xxx
class UpdateSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, UpdateSqlBuilder);

public:
    UpdateSqlBuilder()
    :SqlBuilder(SqlBuilderType::UPDATE)
    {

    }

    ~UpdateSqlBuilder()
    {

    }

    virtual void Release() override
    {
        UpdateSqlBuilder::DeleteThreadLocal_UpdateSqlBuilder(this);
    }

    UpdateSqlBuilder &Clear()
    {
        _kvs.clear();
        _table.clear();
        _where.clear();
        _db.clear();

        return *this;
    }

    UpdateSqlBuilder &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    UpdateSqlBuilder &DB(const LibString &db)
    {
        _db = db;
        return *this;
    }

    UpdateSqlBuilder &Set(const LibString &field, const LibString &value)
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

    UpdateSqlBuilder &Where(const LibString &condition)
    {
        _where = condition;
        return *this;
    }

    LibString Dump() const override
    {
        if(UNLIKELY(_kvs.empty() || _table.empty() || _db.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("UPDATE %s.`%s` SET ", _db.c_str(), _table.c_str());

        const Int32 count = static_cast<Int32>(_kvs.size());
        for(Int32 idx = 0; idx < count; ++idx)
        {
            auto kv = _kvs[idx];
            SqlEscape::escape(kv);
            sql.AppendData(kv);
            
            if((idx + 1) != count)
                sql.AppendFormat(", ");
        }

        if(LIKELY(!_where.empty()))
        {
            sql.AppendFormat(" WHERE ");
            auto cond = _where;
            SqlEscape::escape(cond);
            sql.AppendData(cond);
        }

        return sql;
    }

    LibString ToSql() const override
    {
        if(UNLIKELY(_kvs.empty() || _table.empty() || _db.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("UPDATE %s.`%s` SET ", _db.c_str(), _table.c_str());

        const Int32 count = static_cast<Int32>(_kvs.size());
        for(Int32 idx = 0; idx < count; ++idx)
        {
            sql.AppendData(_kvs[idx]);
            
            if((idx + 1) != count)
                sql.AppendFormat(", ");
        }

        if(LIKELY(!_where.empty()))
        {
            sql.AppendFormat(" WHERE ");
            sql.AppendData(_where);
        }

        return sql;
    }

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::UPDATE kvs:[");
        for(auto &data : _kvs)
        {
            info.AppendData(data);
            info.AppendData(",");
        }
        info.AppendFormat("],\n");
        info.AppendFormat("db:%s\n", _db.c_str());
        info.AppendFormat("table:%s\n", _table.c_str());
        info.AppendFormat("where:");
        info.AppendData(_where);
        
        return info;
    }

private:
    std::vector<LibString> _kvs;
    LibString _db;
    LibString _table;
    LibString _where;
};

// delete from
class DeleteSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, DeleteSqlBuilder);

public:
    DeleteSqlBuilder() 
    :SqlBuilder(SqlBuilderType::DELETE_RECORD)
    {

    }
    ~DeleteSqlBuilder() {}

    virtual void Release() override
    {
        DeleteSqlBuilder::DeleteThreadLocal_DeleteSqlBuilder(this);
    }

    DeleteSqlBuilder &Clear()
    {
        _table.clear();
        _where.clear();
        _db.clear();

        return *this;
    }

    DeleteSqlBuilder &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    DeleteSqlBuilder &DB(const LibString &db)
    {
        _db = db;
        return *this;
    }

    DeleteSqlBuilder &Where(const LibString &condition)
    {
        _where = condition;
        return *this;
    }
    
    LibString Dump() const override
    {
        if(UNLIKELY(_table.empty() || _db.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("DELETE FROM %s.%s", _db.c_str(), _table.c_str());

        if(!_where.empty())
        {
            sql.AppendFormat(" WHERE ");
            auto cond = _where;
            SqlEscape::escape(cond);
            sql.AppendData(_where);
        }

        return sql;
    }

    LibString ToSql() const override
    {
        if(UNLIKELY(_table.empty() || _db.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("DELETE FROM %s.%s", _db.c_str(), _table.c_str());

        if(!_where.empty())
        {
            sql.AppendFormat(" WHERE ");
            sql.AppendData(_where);
        }

        return sql;
    }

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::DELETE_RECORD ");
        info.AppendFormat("db:%s\n", _db.c_str());
        info.AppendFormat("table:%s\n", _table.c_str());
        info.AppendFormat("where:");
        info.AppendData(_where);
        
        return info;
    }

private:
    LibString _db;
    LibString _table;
    LibString _where;
};

// create table if not exists `xxx`() xxx
class CreateTableSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, CreateTableSqlBuilder);

public:
    CreateTableSqlBuilder() 
    :SqlBuilder(SqlBuilderType::CREATE_TABLE)
    {

    }

    ~CreateTableSqlBuilder() 
    {

    }
    
    virtual void Release() override
    {
        CreateTableSqlBuilder::DeleteThreadLocal_CreateTableSqlBuilder(this);
    }

    CreateTableSqlBuilder &Clear()
    {
        _table.clear();
        _fields.clear();
        _engine = "InnoDB";
        _charset = "utf8mb4";
        _collate = "utf8mb4_bin";
        _rowFormat = "DYNAMIC";
        _comment.clear();
        _primaryKey.clear();
        _uniques.clear();
        _indexs.clear();
        _db.clear();

        return *this;
    }

    CreateTableSqlBuilder &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    CreateTableSqlBuilder &DB(const LibString &db)
    {
        _db = db;
        return *this;
    }

    CreateTableSqlBuilder &Field(const LibString &field)
    {
        _fields.push_back(field);
        return *this;
    }
    
    CreateTableSqlBuilder &Engine(const LibString &engine)
    {
        _engine = engine;
        return *this;
    }

    CreateTableSqlBuilder &Charset(const LibString &charset)
    {
        _charset = charset;
        return *this;
    }

    CreateTableSqlBuilder &Collate(const LibString &collate)
    {
        _collate = collate;
        return *this;
    }

    CreateTableSqlBuilder &PrimaryKey(const LibString &field)
    {
        _primaryKey = field;
        return *this;
    }

    CreateTableSqlBuilder &Unique(const LibString &keyName, const std::vector<LibString> &fields)
    {
        if(_uniques.find(keyName) != _uniques.end())
            return *this;

        _uniques.insert(std::make_pair(keyName, fields));
        return *this;
    }

    CreateTableSqlBuilder &Index(const LibString &keyName, const std::vector<LibString> &fields)
    {
        if(_indexs.find(keyName) != _indexs.end())
            return *this;

        _indexs.insert(std::make_pair(keyName, fields));
        return *this;
    }

    CreateTableSqlBuilder &Comment(const LibString &content)
    {
        _comment = content;
        return *this;
    }

    LibString Dump() const override
    {
       if(UNLIKELY(_table.empty() || _fields.empty() || _engine.empty() || _charset.empty() || _collate.empty() || _db.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("CREATE TABLE IF NOT EXISTS %s.`%s`(", _db.c_str(), _table.c_str());

        {
            const Int32 count = static_cast<Int32>(_fields.size());
            for(Int32 idx = 0; idx < count; ++idx)
            {
                sql.AppendData(_fields[idx]);
                
                if((idx + 1) != count)
                    sql.AppendFormat(",");
            } 
        }

        // 主键
        if(!_primaryKey.empty())
        {
            sql.AppendFormat(",PRIMARY KEY(");
            sql.AppendData(_primaryKey);
            sql.AppendFormat(")");
        }

        // 唯一索引
        if(!_uniques.empty())
        {
            sql.AppendFormat(",");
            const Int32 countIter = static_cast<Int32>(_uniques.size());
            Int32 loop = 0;
            for(auto &iter : _uniques)
            {
                auto &keyName = iter.first;
                auto &fields = iter.second;

                sql.AppendFormat("UNIQUE KEY `%s` (", keyName.c_str());
                const Int32 count = static_cast<Int32>(fields.size());
                for(Int32 idx = 0; idx < count; ++idx)
                {
                    sql.AppendFormat("`");
                    sql.AppendData(fields[idx]);
                    sql.AppendFormat("`");
                    
                    if((idx + 1) != count)
                        sql.AppendFormat(",");
                }

                sql.AppendFormat(")");
                if(++loop != countIter)
                    sql.AppendFormat(",");
            }
        }

        // 普通索引
        if(!_indexs.empty())
        {
            sql.AppendFormat(",");
            const Int32 countIter = static_cast<Int32>(_indexs.size());
            Int32 loop = 0;
            for(auto &iter : _indexs)
            {
                auto &keyName = iter.first;
                auto &fields = iter.second;

                sql.AppendFormat("INDEX `%s` (", keyName.c_str());
                const Int32 count = static_cast<Int32>(fields.size());
                for(Int32 idx = 0; idx < count; ++idx)
                {
                    sql.AppendFormat("`");
                    sql.AppendData(fields[idx]);
                    sql.AppendFormat("`");
                    
                    if((idx + 1) != count)
                        sql.AppendFormat(",");
                }

                sql.AppendFormat(")");
                if(++loop != countIter)
                    sql.AppendFormat(",");
            }
        }

        sql.AppendFormat(") ");

        sql.AppendFormat("ENGINE=%s DEFAULT CHARSET=%s, COLLATE = %s", _engine.c_str(), _charset.c_str(), _collate.c_str());

        if(!_rowFormat.empty())
            sql.AppendFormat(", ROW_FORMAT = %s", _rowFormat.c_str());

        if(!_comment.empty())
        {
            auto comment = _comment;
            SqlEscape::escape(comment);
            sql.AppendFormat(", COMMENT = \"").AppendData(comment).AppendFormat("\"");
        }

        return sql;
    }

    LibString ToSql() const override
    {
        if(UNLIKELY(_table.empty() || _fields.empty() || _engine.empty() || _charset.empty() || _collate.empty() || _db.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("CREATE TABLE IF NOT EXISTS %s.`%s`(", _db.c_str(), _table.c_str());

        {
            const Int32 count = static_cast<Int32>(_fields.size());
            for(Int32 idx = 0; idx < count; ++idx)
            {
                sql.AppendData(_fields[idx]);
                
                if((idx + 1) != count)
                    sql.AppendFormat(",");
            } 
        }

        // 主键
        if(!_primaryKey.empty())
        {
            sql.AppendFormat(",PRIMARY KEY(");
            sql.AppendData(_primaryKey);
            sql.AppendFormat(")");
        }

        // 唯一索引
        if(!_uniques.empty())
        {
            sql.AppendFormat(",");
            const Int32 countIter = static_cast<Int32>(_uniques.size());
            Int32 loop = 0;
            for(auto &iter : _uniques)
            {
                auto &keyName = iter.first;
                auto &fields = iter.second;

                sql.AppendFormat("UNIQUE KEY `%s` (", keyName.c_str());
                const Int32 count = static_cast<Int32>(fields.size());
                for(Int32 idx = 0; idx < count; ++idx)
                {
                    sql.AppendFormat("`");
                    sql.AppendData(fields[idx]);
                    sql.AppendFormat("`");
                    
                    if((idx + 1) != count)
                        sql.AppendFormat(",");
                }

                sql.AppendFormat(")");
                if(++loop != countIter)
                    sql.AppendFormat(",");
            }
        }

        // 普通索引
        if(!_indexs.empty())
        {
            sql.AppendFormat(",");
            const Int32 countIter = static_cast<Int32>(_indexs.size());
            Int32 loop = 0;
            for(auto &iter : _indexs)
            {
                auto &keyName = iter.first;
                auto &fields = iter.second;

                sql.AppendFormat("INDEX `%s` (", keyName.c_str());
                const Int32 count = static_cast<Int32>(fields.size());
                for(Int32 idx = 0; idx < count; ++idx)
                {
                    sql.AppendFormat("`");
                    sql.AppendData(fields[idx]);
                    sql.AppendFormat("`");
                    
                    if((idx + 1) != count)
                        sql.AppendFormat(",");
                }

                sql.AppendFormat(")");
                if(++loop != countIter)
                    sql.AppendFormat(",");
            }
        }

        sql.AppendFormat(") ");

        sql.AppendFormat("ENGINE=%s DEFAULT CHARSET=%s, COLLATE = %s", _engine.c_str(), _charset.c_str(), _collate.c_str());

        if(!_rowFormat.empty())
            sql.AppendFormat(", ROW_FORMAT = %s", _rowFormat.c_str());

        if(!_comment.empty())
            sql.AppendFormat(", COMMENT = \"%s\"", _comment.c_str());

        return sql;
    }

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::CREATE_TABLE db:%s \n", _db.c_str());
        info.AppendFormat("table:%s\n", _table.c_str());
        info.AppendFormat("fields:[");
        for(auto &data : _fields)
        {
            info.AppendData(data);
            info.AppendData(",");
        }
        info.AppendFormat("]\n");
        info.AppendFormat("engine:%s\n", _engine.c_str());
        info.AppendFormat("charset:%s\n", _charset.c_str());
        info.AppendFormat("collate:%s\n", _collate.c_str());
        info.AppendFormat("rowFormat:%s\n", _rowFormat.c_str());
        info.AppendFormat("comment:");
        info.AppendData(_comment);
        info.AppendData("\n");
        info.AppendFormat("_primaryKey:%s\n", _primaryKey.c_str());

        info.AppendFormat("uniques:\n{");
        for(auto iter : _uniques)
        {
            auto &uniqueName = iter.first;
            info.AppendFormat("unique name:");
            info.AppendData(uniqueName);
            info.AppendFormat("[");
            auto &uniqueKeys = iter.second;
            for(auto &key : uniqueKeys)
            {
                info.AppendData(key);
                info.AppendFormat(",");
            }
            info.AppendFormat("]");
        }
        info.AppendFormat("}\n");
        
        info.AppendFormat("indexs:\n{");
        for(auto iter : _indexs)
        {
            auto &indexName = iter.first;
            info.AppendFormat("index name:");
            info.AppendData(indexName);
            info.AppendFormat("[");
            auto &indexKeys = iter.second;
            for(auto &key : indexKeys)
            {
                info.AppendData(key);
                info.AppendFormat(",");
            }
            info.AppendFormat("]");
        }
        info.AppendFormat("}\n");

        return info;
    }

private:
    LibString _db;
    LibString _table;
    std::vector<LibString> _fields;
    LibString _engine = "InnoDB";
    LibString _charset = "utf8mb4";
    LibString _collate = "utf8mb4_bin";
    LibString _rowFormat = "DYNAMIC";
    LibString _comment;
    LibString _primaryKey;
    std::map<LibString, std::vector<LibString>> _uniques;
    std::map<LibString, std::vector<LibString>> _indexs;
};

// truncate table `xxx`
class TruncateTableSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, TruncateTableSqlBuilder);

public:
    TruncateTableSqlBuilder() 
    :SqlBuilder(SqlBuilderType::TRUNCATE_TABLE)
    {

    }

    ~TruncateTableSqlBuilder() 
    {

    }

    virtual void Release() override
    {
        TruncateTableSqlBuilder::DeleteThreadLocal_TruncateTableSqlBuilder(this);
    }

    TruncateTableSqlBuilder &Clear()
    {
        _table.clear();
        _db.clear();

        return *this;
    }

    TruncateTableSqlBuilder &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }
    
    TruncateTableSqlBuilder &DB(const LibString &db)
    {
        _db = db;
        return *this;
    }

    LibString Dump() const override
    {
        return ToSql();
    }

    LibString ToSql() const override
    {
        if(UNLIKELY(_table.empty() || _db.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("TRUNCATE TABLE %s.`%s`", _db.c_str(), _table.c_str());
        return sql;
    }

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::TRUNCATE_TABLE db:%s \n", _db.c_str());
        info.AppendFormat("table:%s\n", _table.c_str());

        return info;
    }

private:
    LibString _db;
    LibString _table;
};

// drop table if exists `xxx`
class DropTableSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, DropTableSqlBuilder);

public:
    DropTableSqlBuilder() 
    :SqlBuilder(SqlBuilderType::DROP_TABLE)
    {

    }

    ~DropTableSqlBuilder() 
    {

    }

    virtual void Release() override
    {
        DropTableSqlBuilder::DeleteThreadLocal_DropTableSqlBuilder(this);
    }

    DropTableSqlBuilder &Clear()
    {
        _table.clear();
        _db.clear();

        return *this;
    }

    DropTableSqlBuilder &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }
    
    DropTableSqlBuilder &DB(const LibString &db)
    {
        _db = db;
        return *this;
    }

    LibString Dump() const override
    {
        return ToSql();
    }

    LibString ToSql() const override
    {
        if(UNLIKELY(_table.empty() || _db.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("DROP TABLE IF EXISTS %s.`%s`", _db.c_str(), _table.c_str());
        return sql;
    }

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::DROP_TABLE db:%s \n", _db.c_str());
        info.AppendFormat("table:%s\n", _table.c_str());

        return info;
    }

private:
    LibString _db;
    LibString _table;
};

// create DATABASE if not exists `xxx`
class CreateDBSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, CreateDBSqlBuilder);

public:
    CreateDBSqlBuilder() 
    :SqlBuilder(SqlBuilderType::CREATE_DB)
    {

    }

    ~CreateDBSqlBuilder() 
    {

    }

    virtual void Release() override
    {
        CreateDBSqlBuilder::DeleteThreadLocal_CreateDBSqlBuilder(this);
    }

    CreateDBSqlBuilder &Clear()
    {
        _db.clear();
        _charset = "utf8mb4";
        _collate = "utf8mb4_bin";

        return *this;
    }

    CreateDBSqlBuilder &DB(const LibString &db)
    {
        _db = db;
        return *this;
    }

    CreateDBSqlBuilder &Charset(const LibString &charset)
    {
        _charset = charset;
        return *this;
    }

    CreateDBSqlBuilder &Collate(const LibString &collate)
    {
        _collate = collate;
        return *this;
    }

    LibString Dump() const override
    {
        return ToSql();
    }

    LibString ToSql() const override
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

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::CREATE_DB db:%s \n", _db.c_str());
        info.AppendFormat("charset:%s \n", _charset.c_str());
        info.AppendFormat("collate:%s \n", _collate.c_str());

        return info;
    }

private:
    LibString _db;
    LibString _charset = "utf8mb4";
    LibString _collate = "utf8mb4_bin";
};

// create DATABASE if not exists `xxx`
class DropDBSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, DropDBSqlBuilder);

public:
    DropDBSqlBuilder() 
    :SqlBuilder(SqlBuilderType::DROP_DB)
    {

    }

    ~DropDBSqlBuilder() 
    {

    }

    virtual void Release() override
    {
        DropDBSqlBuilder::DeleteThreadLocal_DropDBSqlBuilder(this);
    }

    DropDBSqlBuilder &Clear()
    {
        _db.clear();

        return *this;
    }

    DropDBSqlBuilder &DB(const LibString &db)
    {
        _db = db;
        return *this;
    }

    LibString Dump() const override
    {
        return ToSql();
    }

    LibString ToSql() const override
    {
        if(UNLIKELY(_db.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("DROP DATABASE IF EXISTS `%s`", _db.c_str());

        return sql;
    }
    
    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::DROP_DB db:%s \n", _db.c_str());

        return info;
    }

private:
    LibString _db;
};

// ALTER TABLE `xxx` ADD/DROP/MODIFY COLUMN ...
class AlterTableSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, AlterTableSqlBuilder);

public:
    AlterTableSqlBuilder() 
    :SqlBuilder(SqlBuilderType::ALTER_TABLE)
    {

    }
    ~AlterTableSqlBuilder() 
    {

    }

    virtual void Release() override
    {
        AlterTableSqlBuilder::DeleteThreadLocal_AlterTableSqlBuilder(this);
    }

    AlterTableSqlBuilder &Clear()
    {
        _adds.clear();
        _renames.clear();
        _modifys.clear();
        _drops.clear();
        _addIndexs.clear();
        _addUniqueIndexs.clear();
        _dropIndexs.clear();
        _addPrimaryKeys.clear();
        _changeEngine.clear();
        _type = CHANGE_UNKNOWN;
        _table.clear();
        _db.clear();

        return *this;
    }

    AlterTableSqlBuilder &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    AlterTableSqlBuilder &DB(const LibString &db)
    {
        _db = db;
        return *this;
    }

    AlterTableSqlBuilder &Add(const std::vector<std::tuple<LibString, LibString, LibString>> &fields)
    {
        for(auto &tupleInfo : fields)
        {
            auto &field = std::get<0>(tupleInfo);
            auto &desc = std::get<1>(tupleInfo);
            if(UNLIKELY(field.empty() || desc.empty()))
                continue;

            _adds.push_back(tupleInfo);
        }

        if(UNLIKELY(_adds.empty()))
            return *this;

        _type = CHANGE_ADD;
        return *this;
    }

    AlterTableSqlBuilder &Add(const LibString &field, const LibString &desc, const LibString &after = "")
    {
        if(UNLIKELY(field.empty() || desc.empty()))
            return *this;

        _adds.push_back(std::make_tuple(field, desc, after));
        _type = CHANGE_ADD;
        return *this;
    }

    AlterTableSqlBuilder &Rename(const LibString &oldField, const LibString &newField)
    {
        if(UNLIKELY(oldField.empty() || newField.empty()))
            return *this;

        _renames.push_back(std::make_tuple(oldField, newField));

        _type = CHANGE_RENAME;
        return *this;
    }

    AlterTableSqlBuilder &Rename(const std::vector<std::tuple<LibString, LibString>> &renameInfoList)
    {
        const Int32 count = static_cast<Int32>(renameInfoList.size());
        for(Int32 idx = 0; idx < count; ++idx)
        {
            auto &renameInfo = renameInfoList[idx];
            auto &oldField = std::get<0>(renameInfo);
            auto &newField = std::get<1>(renameInfo);
            if(UNLIKELY(oldField.empty() || newField.empty()))
                continue;

            _renames.push_back(renameInfo);
        }

        if(UNLIKELY(_renames.empty()))
            return *this;

        _type = CHANGE_RENAME;
        return *this;
    }

    AlterTableSqlBuilder &Modify(const LibString &field, const LibString &desc)
    {
        if(UNLIKELY(field.empty() || desc.empty()))
            return *this;

        _modifys.push_back(std::make_tuple(field, desc));

        _type = CHANGE_MODIFY;
        return *this;
    }

    AlterTableSqlBuilder &Modify(const std::vector<std::tuple<LibString, LibString>> &contents)
    {
        for(auto &tupleInfo : contents)
        {
            auto &field = std::get<0>(tupleInfo);
            auto &desc = std::get<1>(tupleInfo);
            if(UNLIKELY(field.empty() || desc.empty()))
                continue;

            _modifys.push_back(tupleInfo);
        }

        _type = CHANGE_MODIFY;
        return *this;
    }

    AlterTableSqlBuilder &Drop(const LibString &field)
    {
        if(UNLIKELY(field.empty()))
            return *this;

        _drops.push_back(field);
        _type = CHANGE_DROP;
        return *this;
    }

    AlterTableSqlBuilder &Drop(const std::vector<LibString> &fields)
    {
        for(auto &field : fields)
        {
            if(UNLIKELY(field.empty()))
                continue;

            _drops.push_back(field);
        }

        if(UNLIKELY(_drops.empty()))
            return *this;

        _type = CHANGE_DROP;
        return *this;
    }

    AlterTableSqlBuilder &AddIndex(const LibString &indexName, const std::vector<LibString> &fields, const LibString &usingDesc, const LibString &comment, bool usingFulltext = false, const LibString &fulltextWithParser = "")
    {
        if(UNLIKELY(indexName.empty() || fields.empty()))
            return *this;

        const auto &content = std::make_tuple(indexName, fields, usingDesc, comment, usingFulltext, fulltextWithParser);
        _addIndexs.push_back(content);
        _type = CHANGE_ADD_INDEX;
        return *this;
    }

    AlterTableSqlBuilder &AddIndex(const std::vector<std::tuple<LibString, std::vector<LibString>, LibString, LibString, bool, LibString>> &contents)
    {
        for(auto &content : contents)
        {
            const auto &indexName = std::get<0>(content);
            const auto &fields = std::get<1>(content);
            if(UNLIKELY(indexName.empty() || fields.empty()))
                continue;

            _addIndexs.push_back(content);
        }

        if(UNLIKELY(_addIndexs.empty()))
            return *this;

        _type = CHANGE_ADD_INDEX;
        return *this;     
    }

    AlterTableSqlBuilder &AddUniqueIndex(const LibString &indexName, const std::vector<LibString> &fields, const LibString &usingDesc, const LibString &comment)
    {
        if(UNLIKELY(indexName.empty() || fields.empty()))
            return *this;

        const auto &content = std::make_tuple(indexName, fields, usingDesc, comment);
        _addUniqueIndexs.push_back(content);
        _type = CHANGE_ADD_UNIQUE_INDEX;
        return *this;
    }

    AlterTableSqlBuilder &AddUniqueIndex(const std::vector<std::tuple<LibString, std::vector<LibString>, LibString, LibString>> &contents)
    {
        for(auto &content : contents)
        {
            const auto &indexName = std::get<0>(content);
            const auto &fields = std::get<1>(content);
            if(UNLIKELY(indexName.empty() || fields.empty()))
                continue;

            _addUniqueIndexs.push_back(content);
        }

        if(UNLIKELY(_addUniqueIndexs.empty()))
            return *this;

        _type = CHANGE_ADD_UNIQUE_INDEX;
        return *this;     
    }

    AlterTableSqlBuilder &DropIndex(const LibString &indexName)
    {
        if(UNLIKELY(indexName.empty()))
            return *this;

        _dropIndexs.push_back(indexName);
        _type = CHANGE_DROP_INDEX;
        return *this;
    }

    AlterTableSqlBuilder &DropIndex(const std::vector<LibString> &indexNames)
    {
        for(auto &indexName : indexNames)
        {
            if(UNLIKELY(indexName.empty()))
                continue;

            _dropIndexs.push_back(indexName);
        }

        if(UNLIKELY(_dropIndexs.empty()))
            return *this;

        _type = CHANGE_DROP_INDEX;
        return *this;
    }

    AlterTableSqlBuilder &AddPrimaryKey(const LibString &field)
    {
        if(UNLIKELY(field.empty()))
            return *this;
        
        _addPrimaryKeys.push_back(field);
        _type = CHANGE_ADD_PRIMARY_KEY;
        return *this;
    }

    AlterTableSqlBuilder &AddPrimaryKey(const std::vector<LibString> &fields)
    {
        if(UNLIKELY(fields.empty()))
            return *this;

        for(auto &field : fields)
            _addPrimaryKeys.push_back(field);

        if(UNLIKELY(_addPrimaryKeys.empty()))
            return *this;

        _type = CHANGE_ADD_PRIMARY_KEY;
        return *this;
    }

    AlterTableSqlBuilder &DropPrimaryKey()
    {
        _type = CHANGE_DROP_PRIMARY_KEY;
        return *this;
    }

    AlterTableSqlBuilder &ChangeEngine(const LibString &engine)
    {
        _type = CHANGE_ENGINE;
        _changeEngine = engine;
        return *this;
    }

    LibString Dump() const override
    {
        return ToSql();
    }

    LibString ToSql() const override
    {
        if((_type == CHANGE_UNKNOWN) || 
            (_table.empty() || _db.empty()) )
            return "";

        LibString sql;
        switch (_type)
        {
        case CHANGE_ADD: 
            return _BuildAddSql();
        case CHANGE_RENAME:
            return _BuildRenameSql();
        case CHANGE_DROP:
            return _BuildDropSql();
        case CHANGE_MODIFY:
            return _BuildModifySql();
        case CHANGE_ADD_INDEX:
            return _BuildAddIndexSql();
        case CHANGE_ADD_UNIQUE_INDEX:
            return _BuildAddUniqueIndexSql();
        case CHANGE_DROP_INDEX:
            return _BuildDropIndexSql();
        case CHANGE_ADD_PRIMARY_KEY:
            return _BuildAddPrimaryKeySql();
        case CHANGE_DROP_PRIMARY_KEY:
            return _BuildDropPrimaryKeySql();
        case CHANGE_ENGINE:
            return _BuildChangeEngineSql();
        default:
            break;
        }

        return "";
    }

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::ALTER_TABLE db:%s \n", _db.c_str());
        info.AppendFormat("table:%s \n", _table.c_str());
        info.AppendFormat("type:%d \n", _type);
        info.AppendFormat("changeEngine:").AppendData(_changeEngine).AppendFormat("\n");

        info.AppendFormat("addPrimaryKeys:[");
        for(auto &key : _addPrimaryKeys)
            info.AppendData(key).AppendFormat(",");
        info.AppendFormat("]\n");

        info.AppendFormat("dropIndexs:[");
        for(auto &key : _dropIndexs)
            info.AppendData(key).AppendFormat(",");
        info.AppendFormat("]\n");

        info.AppendFormat("addUniqueIndexs:[");
        for(auto &uniqueIndexInfo : _addUniqueIndexs)
        {
            info.AppendFormat("index name:").AppendData(std::get<0>(uniqueIndexInfo)).AppendFormat(", ");

            info.AppendFormat("column list:[");
            auto &columnList = std::get<1>(uniqueIndexInfo);
            for(auto &column : columnList)
                info.AppendData(column).AppendFormat(",");
            info.AppendFormat("], ");

            info.AppendFormat("using tree:").AppendData(std::get<2>(uniqueIndexInfo)).AppendFormat(",");
            info.AppendFormat("comment:").AppendData(std::get<3>(uniqueIndexInfo)).AppendFormat(",");

            info.AppendFormat(";");
        }
        info.AppendFormat("]\n");

        info.AppendFormat("addIndexs:[");
        for(auto &addIndexInfo : _addIndexs)
        {
            info.AppendFormat("index name:").AppendData(std::get<0>(addIndexInfo)).AppendFormat(", ");

            info.AppendFormat("column list:[");
            auto &columnList = std::get<1>(addIndexInfo);
            for(auto &column : columnList)
                info.AppendData(column).AppendFormat(",");
            info.AppendFormat("], ");

            info.AppendFormat("using tree:").AppendData(std::get<2>(addIndexInfo)).AppendFormat(",");
            info.AppendFormat("comment:").AppendData(std::get<3>(addIndexInfo)).AppendFormat(",");
            info.AppendFormat("is fulltext:").AppendData(std::get<4>(addIndexInfo) ? "true" : "false").AppendFormat(",");
            info.AppendFormat("fulltext parser:").AppendData(std::get<5>(addIndexInfo)).AppendFormat(",");

            info.AppendFormat(";");
        }
        info.AppendFormat("]\n");

        info.AppendFormat("drops:[");
        for(auto &key : _drops)
            info.AppendData(key).AppendFormat(",");
        info.AppendFormat("]\n");

        info.AppendFormat("modifys:[");
        for(auto &modifyInfo : _modifys)
        {
            info.AppendFormat("field:").AppendData(std::get<0>(modifyInfo)).AppendFormat(", ");
            info.AppendFormat("desc:").AppendData(std::get<1>(modifyInfo)).AppendFormat(", ");

            info.AppendFormat(";");
        }
        info.AppendFormat("]\n");

        info.AppendFormat("renames:[");
        for(auto &renameInfo : _renames)
        {
            info.AppendFormat("old field:").AppendData(std::get<0>(renameInfo)).AppendFormat(", ");
            info.AppendFormat("new field:").AppendData(std::get<1>(renameInfo)).AppendFormat(", ");

            info.AppendFormat(";");
        }
        info.AppendFormat("]\n");

        info.AppendFormat("adds:[");
        for(auto &addInfo : _adds)
        {
            info.AppendFormat("field:").AppendData(std::get<0>(addInfo)).AppendFormat(", ");
            info.AppendFormat("desc:").AppendData(std::get<1>(addInfo)).AppendFormat(", ");
            info.AppendFormat("after:").AppendData(std::get<2>(addInfo)).AppendFormat(", ");

            info.AppendFormat(";");
        }
        info.AppendFormat("]\n");

        return info;
    }

private:
    LibString _BuildAddSql() const
    {
        if(UNLIKELY(_adds.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("ALTER TABLE %s.`%s`", _db.c_str(), _table.c_str());
        const Int32 count = static_cast<Int32>(_adds.size());
        bool hasChanged = false;
        for(Int32 idx = 0; idx < count; ++idx)
        {
            auto &addInfo = _adds[idx];
            auto &field = std::get<0>(addInfo);
            auto &desc = std::get<1>(addInfo);
            auto &afterField = std::get<2>(addInfo);

            if(field.empty() || desc.empty())
                continue;
            
            hasChanged = true;
            sql.AppendFormat(" ADD COLUMN `%s` ", field.c_str());
            sql.AppendData(desc);

            if(!afterField.empty())
                sql.AppendFormat(" AFTER `%s`", afterField.c_str());

            if((idx + 1) != count)
                sql.AppendFormat(",");
        }

        if(!hasChanged)
            return "";

        return sql;
    }

    LibString _BuildRenameSql() const
    {
        if(UNLIKELY(_renames.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("ALTER TABLE %s.`%s`", _db.c_str(), _table.c_str());
        const Int32 count = static_cast<Int32>(_renames.size());
        bool hasChanged = false;
        for(Int32 idx = 0; idx < count; ++idx)
        {
            auto &tupleInfo = _renames[idx];
            auto &oldField = std::get<0>(tupleInfo);
            auto &newField = std::get<1>(tupleInfo);

            if(oldField.empty() || newField.empty())
                continue;
            
            hasChanged = true;
            sql.AppendFormat(" RENAME COLUMN `%s` TO `%s` ", oldField.c_str(), newField.c_str());

            if((idx + 1) != count)
                sql.AppendFormat(",");
        }

        if(!hasChanged)
            return "";

        return sql;
    }

    LibString _BuildDropSql() const
    {
        if(UNLIKELY(_drops.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("ALTER TABLE %s.`%s`", _db.c_str(), _table.c_str());
        const Int32 count = static_cast<Int32>(_drops.size());
        bool hasChanged = false;
        for(Int32 idx = 0; idx < count; ++idx)
        {
            auto &field = _drops[idx];

            if(field.empty())
                continue;
            
            hasChanged = true;
            sql.AppendFormat(" DROP COLUMN `%s` ", field.c_str());

            if((idx + 1) != count)
                sql.AppendFormat(",");
        }

        if(!hasChanged)
            return "";

        return sql;
    }

    LibString _BuildModifySql() const
    {
        if(UNLIKELY(_modifys.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("ALTER TABLE %s.`%s`", _db.c_str(), _table.c_str());
        const Int32 count = static_cast<Int32>(_modifys.size());
        bool hasChanged = false;
        for(Int32 idx = 0; idx < count; ++idx)
        {
            auto &tupleInfo = _modifys[idx];
            auto &field = std::get<0>(tupleInfo);
            auto &desc = std::get<1>(tupleInfo);

            if(field.empty() || desc.empty())
                continue;
            
            hasChanged = true;
            sql.AppendFormat(" MODIFY COLUMN `%s` ", field.c_str());
            sql.AppendData(desc);

            if((idx + 1) != count)
                sql.AppendFormat(",");
        }

        if(!hasChanged)
            return "";

        return sql;
    }

    LibString _BuildAddIndexSql() const
    {
        if(UNLIKELY(_addIndexs.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("ALTER TABLE %s.`%s`", _db.c_str(), _table.c_str());
        const Int32 count = static_cast<Int32>(_addIndexs.size());
        bool hasChanged = false;
        for(Int32 idx = 0; idx < count; ++idx)
        {
            auto &addInfo = _addIndexs[idx];

            auto &indexName = std::get<0>(addInfo);
            auto &fields = std::get<1>(addInfo);
            auto &usingDesc = std::get<2>(addInfo);
            auto &comment = std::get<3>(addInfo);
            auto &usingFulltext = std::get<4>(addInfo);
            auto &fulltextWithParser = std::get<5>(addInfo);

            if(UNLIKELY(indexName.empty() || fields.empty()))
                continue;
            
            hasChanged = true;
            if(usingFulltext)
            {
                sql.AppendFormat(" ADD FULLTEXT INDEX `%s`(", indexName.c_str());
            }
            else
            {
                sql.AppendFormat(" ADD INDEX `%s`(", indexName.c_str());
            }
            sql.AppendData(StringUtil::ToString(fields, ","))
                .AppendData(")");

            // 全文索引不能使用btree等
            if(!usingFulltext && !usingDesc.empty())
                sql.AppendFormat(" ").AppendData(usingDesc);

            // 全文索引使用的解析器
            if(!fulltextWithParser.empty())
                sql.AppendFormat(" %s", fulltextWithParser.c_str());

            if(!comment.empty())
                sql.AppendFormat(" COMMENT '").AppendData(comment).AppendData("'");

            if((idx + 1) != count)
                sql.AppendFormat(",");
        }

        if(!hasChanged)
            return "";

        return sql;
    }

    LibString _BuildAddUniqueIndexSql() const
    {
        if(UNLIKELY(_addUniqueIndexs.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("ALTER TABLE %s.`%s`", _db.c_str(), _table.c_str());
        const Int32 count = static_cast<Int32>(_addUniqueIndexs.size());
        bool hasChanged = false;
        for(Int32 idx = 0; idx < count; ++idx)
        {
            auto &addInfo = _addUniqueIndexs[idx];

            auto &indexName = std::get<0>(addInfo);
            auto &fields = std::get<1>(addInfo);
            auto &usingDesc = std::get<2>(addInfo);
            auto &comment = std::get<3>(addInfo);

            if(UNLIKELY(indexName.empty() || fields.empty()))
                continue;
            
            hasChanged = true;

            sql.AppendFormat(" ADD UNIQUE INDEX `%s`(", indexName.c_str());
            sql.AppendData(StringUtil::ToString(fields, ","))
                .AppendData(")");

            // 全文索引不能使用btree等
            if(!usingDesc.empty())
                sql.AppendFormat(" ").AppendData(usingDesc);

            if(!comment.empty())
                sql.AppendFormat(" COMMENT '").AppendData(comment).AppendData("'");

            if((idx + 1) != count)
                sql.AppendFormat(",");
        }

        if(!hasChanged)
            return "";

        return sql;
    }
    
    LibString _BuildDropIndexSql() const
    {
        if(UNLIKELY(_dropIndexs.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("ALTER TABLE %s.`%s`", _db.c_str(), _table.c_str());
        const Int32 count = static_cast<Int32>(_dropIndexs.size());
        bool hasChanged = false;
        for(Int32 idx = 0; idx < count; ++idx)
        {
            auto &indexName = _dropIndexs[idx];

            if(indexName.empty())
                continue;
            
            hasChanged = true;
            sql.AppendFormat(" DROP INDEX `%s` ", indexName.c_str());

            if((idx + 1) != count)
                sql.AppendFormat(",");
        }

        if(!hasChanged)
            return "";

        return sql;
    }

    LibString _BuildAddPrimaryKeySql() const
    {
        if(UNLIKELY(_addPrimaryKeys.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("ALTER TABLE %s.`%s` ADD PRIMARY KEY (%s)", _db.c_str(), _table.c_str(), StringUtil::ToString(_addPrimaryKeys, ",").c_str());
        return sql;
    }

    LibString _BuildDropPrimaryKeySql() const
    {
        LibString sql;
        sql.AppendFormat("ALTER TABLE %s.`%s` DROP PRIMARY KEY", _db.c_str(), _table.c_str());
        return sql;
    }

    LibString _BuildChangeEngineSql() const
    {
        LibString sql;
        sql.AppendFormat("ALTER TABLE %s.`%s` ENGINE = %s", _db.c_str(), _table.c_str(), _changeEngine.c_str());
        return sql;
    }

private:

    // 类型
    enum ChangeType
    {
        CHANGE_UNKNOWN = 0,
        CHANGE_ADD,
        CHANGE_RENAME,
        CHANGE_DROP,
        CHANGE_MODIFY,
        CHANGE_ADD_INDEX,
        CHANGE_ADD_UNIQUE_INDEX,
        CHANGE_DROP_INDEX,
        CHANGE_ADD_PRIMARY_KEY,
        CHANGE_DROP_PRIMARY_KEY,
        CHANGE_ENGINE,
    };

    std::vector<std::tuple<LibString, LibString, LibString>> _adds; // field, desc, after
    std::vector<std::tuple<LibString, LibString>> _renames;         // oldField, newField
    std::vector<std::tuple<LibString, LibString>> _modifys;         // field, desc
    std::vector<LibString> _drops;                                  // field
    std::vector<std::tuple<LibString, std::vector<LibString>, LibString, LibString, bool, LibString>> _addIndexs; // index name, column name list, using btree等, comment, 是否使用FULLTEXT, fulltext with parser(建立全文索引并使用解析器,建议ngram)     
    std::vector<std::tuple<LibString, std::vector<LibString>, LibString, LibString>> _addUniqueIndexs; // index name, column name list, using btree等, comment
    std::vector<LibString> _dropIndexs;
    std::vector<LibString> _addPrimaryKeys;
    LibString _changeEngine;
    Int32 _type = CHANGE_UNKNOWN;
    LibString _table;
    LibString _db;
};

// ALTER TABLE `xxx` ADD/DROP/MODIFY COLUMN ...
class ShowIndexSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, ShowIndexSqlBuilder);

public:
    ShowIndexSqlBuilder() 
    :SqlBuilder(SqlBuilderType::SHOW_INDEX)
    {

    }

    ~ShowIndexSqlBuilder() 
    {

    }

    virtual void Release() override
    {
        ShowIndexSqlBuilder::DeleteThreadLocal_ShowIndexSqlBuilder(this);
    }

    ShowIndexSqlBuilder &Clear()
    {
        _table.clear();
        _db.clear();

        return *this;
    }

    ShowIndexSqlBuilder &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    ShowIndexSqlBuilder &DB(const LibString &db)
    {
        _db = db;
        return *this;
    }

    LibString Dump() const override
    {
        return ToSql();
    }

    LibString ToSql() const override
    {
        if(UNLIKELY(_table.empty() || _db.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("SHOW INDEX FROM %s.`%s`", _db.c_str(), _table.c_str());
        return sql;
    }

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::SHOW_INDEX db:%s, ", _db.c_str());
        info.AppendFormat("table:%s \n", _table.c_str());

        return info;
    }

private:
    LibString _db;
    LibString _table;
};

// ALTER TABLE `xxx` ADD/DROP/MODIFY COLUMN ...
class OptimizeTableSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, OptimizeTableSqlBuilder);

public:
    OptimizeTableSqlBuilder() 
    :SqlBuilder(SqlBuilderType::OPTIMIZE_TABLE)
    {

    }

    ~OptimizeTableSqlBuilder() 
    {

    }  

    virtual void Release() override
    {
        OptimizeTableSqlBuilder::DeleteThreadLocal_OptimizeTableSqlBuilder(this);
    }

    OptimizeTableSqlBuilder &Clear()
    {
        _table.clear();
        _db.clear();

        return *this;
    }

    OptimizeTableSqlBuilder &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    OptimizeTableSqlBuilder &DB(const LibString &db)
    {
        _db = db;
        return *this;
    }

    LibString Dump() const override
    {
        return ToSql();
    }

    LibString ToSql() const override
    {
        if(UNLIKELY(_table.empty() || _db.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("OPTIMIZE TABLE %s.`%s`", _db.c_str(), _table.c_str());
        return sql;
    }

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::OPTIMIZE_TABLE db:%s, ", _db.c_str());
        info.AppendFormat("table:%s \n", _table.c_str());

        return info;
    }
private:
    LibString _table;
    LibString _db;
};

class StartTransActionSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, StartTransActionSqlBuilder);

public:
    StartTransActionSqlBuilder() 
    :SqlBuilder(SqlBuilderType::START_TRANSACTION)
    {

    }

    ~StartTransActionSqlBuilder() 
    {

    }  

    virtual void Release() override
    {
        StartTransActionSqlBuilder::DeleteThreadLocal_StartTransActionSqlBuilder(this);
    }

    StartTransActionSqlBuilder &Clear()
    {
        return *this;
    }

    LibString Dump() const override
    {
        return ToSql();
    }

    LibString ToSql() const override
    {
        return "START TRANSACTION";
    }

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::START_TRANSACTION");

        return info;
    }
};

class SetAutoCommitSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, SetAutoCommitSqlBuilder);

public:
    SetAutoCommitSqlBuilder() 
    :SqlBuilder(SqlBuilderType::SET_AUTOCOMMIT)
    {

    }

    ~SetAutoCommitSqlBuilder() 
    {

    }  

    virtual void Release() override
    {
        SetAutoCommitSqlBuilder::DeleteThreadLocal_SetAutoCommitSqlBuilder(this);
    }

    SetAutoCommitSqlBuilder &Clear()
    {
        _autoCommit = true;
        return *this;
    }

    SetAutoCommitSqlBuilder &SetAutoCommit(bool autoCommit = true)
    {
        _autoCommit = autoCommit;
        return *this;
    }

    LibString Dump() const override
    {
        return ToSql();
    }

    LibString ToSql() const override
    {
        return LibString().AppendFormat("set autocommit = %d", _autoCommit ? 1 : 0);
    }

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::SET_AUTOCOMMIT autoCommit:%s", _autoCommit ? "true" : "false");

        return info;
    }

private:
    bool _autoCommit = true;
};

class RollbackSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, RollbackSqlBuilder);

public:
    RollbackSqlBuilder() 
    :SqlBuilder(SqlBuilderType::ROLLBACK)
    {

    }
    ~RollbackSqlBuilder() 
    {

    }  

    virtual void Release() override
    {
        RollbackSqlBuilder::DeleteThreadLocal_RollbackSqlBuilder(this);
    }

    RollbackSqlBuilder &Clear()
    {
        return *this;
    }

    LibString Dump() const override
    {
        return ToSql();
    }

    LibString ToSql() const override
    {
        return "ROLLBACK";
    }

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::ROLLBACK ");

        return info;
    }
};

class CommitTransActionSqlBuilder : public SqlBuilder
{
    POOL_CREATE_OBJ_DEFAULT_P1(SqlBuilder, CommitTransActionSqlBuilder);

public:
    CommitTransActionSqlBuilder() 
    :SqlBuilder(SqlBuilderType::COMMIT_TRANSACTION)
    {

    }

    ~CommitTransActionSqlBuilder() 
    {

    }  

    virtual void Release() override
    {
        CommitTransActionSqlBuilder::DeleteThreadLocal_CommitTransActionSqlBuilder(this);
    }

    CommitTransActionSqlBuilder &Clear()
    {
        return *this;
    }

    LibString Dump() const override
    {
        return ToSql();
    }

    LibString ToSql() const override
    {
        return "COMMIT";
    }

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("SqlBuilderType::COMMIT_TRANSACTION ");

        return info;
    }
};

KERNEL_END

#endif

