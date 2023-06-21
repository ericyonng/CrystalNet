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
#include <kernel/comp/Utils/StringUtil.h>

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

        // 表结构 TODO:
        ALTER_TABLE,
        SHOW_INDEX,
    };
};

// 在全文索引时候指定的解析器
class FullTextParser
{
public:
    static const LibString WITH_PARSER_NGRAM;
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
            sql.AppendFormat("*");
        }

        {// from
            sql.AppendFormat(" FROM ");
            const Int32 count = static_cast<Int32>(_tables.size());
            for(Int32 idx = 0; idx < count; ++idx)
            {
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
        {
            sql.AppendFormat(" WHERE ");
            sql.AppendData(_where);
        }

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
        _primaryKey.clear();
        _uniques.clear();
        _indexs.clear();
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

    SqlBuilder<SqlBuilderType::CREATE_TABLE> &PrimaryKey(const LibString &field)
    {
        _primaryKey = field;
        return *this;
    }

    SqlBuilder<SqlBuilderType::CREATE_TABLE> &Unique(const LibString &keyName, const std::vector<LibString> &fields)
    {
        if(_uniques.find(keyName) != _uniques.end())
            return *this;

        _uniques.insert(std::make_pair(keyName, fields));
        return *this;
    }

    SqlBuilder<SqlBuilderType::CREATE_TABLE> &Index(const LibString &keyName, const std::vector<LibString> &fields)
    {
        if(_indexs.find(keyName) != _indexs.end())
            return *this;

        _indexs.insert(std::make_pair(keyName, fields));
        return *this;
    }

    SqlBuilder<SqlBuilderType::CREATE_TABLE> &Comment(const LibString &content)
    {
        _comment = content;
        return *this;
    }


    LibString ToSql() const
    {
        if(UNLIKELY(_table.empty() || _fields.empty() || _engine.empty() || _charset.empty() || _collate.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("CREATE TABLE IF NOT EXISTS `%s`(", _table.c_str());

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

private:
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

    void Clear()
    {
        _db.clear();
    }

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

// ALTER TABLE `xxx` ADD/DROP/MODIFY COLUMN ...
template<>
class SqlBuilder<SqlBuilderType::ALTER_TABLE>
{
public:
    SqlBuilder() {}
    ~SqlBuilder() {}

    void Clear()
    {
        _adds.clear();
        _renames.clear();
        _modifys.clear();
        _drops.clear();
        _addIndexs.clear();
        _addUniqueIndexs.clear();
        _dropIndexs.clear();
        _addPrimaryKeys.clear();
        _type = CHANGE_UNKNOWN;
        _table.clear();
    }

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &Add(const std::vector<std::tuple<LibString, LibString, LibString>> &fields)
    {
        for(auto &tupleInfo : fields)
        {
            auto &field = std::get<0>(tupleInfo);
            auto &desc = std::get<1>(tupleInfo);
            auto &afterField = std::get<2>(tupleInfo);
            if(UNLIKELY(field.empty() || desc.empty()))
                continue;

            _adds.push_back(tupleInfo);
        }

        if(UNLIKELY(_adds.empty()))
            return *this;

        _type = CHANGE_ADD;
        return *this;
    }

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &Add(const LibString &field, const LibString &desc, const LibString &after = "")
    {
        if(UNLIKELY(field.empty() || desc.empty()))
            return *this;

        _adds.push_back(std::make_tuple(field, desc, after));
        _type = CHANGE_ADD;
        return *this;
    }

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &Rename(const LibString &oldField, const LibString &newField)
    {
        if(UNLIKELY(oldField.empty() || newField.empty()))
            return *this;

        _renames.push_back(std::make_tuple(oldField, newField));

        _type = CHANGE_RENAME;
        return *this;
    }

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &Rename(const std::vector<std::tuple<LibString, LibString>> &renameInfoList)
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

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &Modify(const LibString &field, const LibString &desc)
    {
        if(UNLIKELY(field.empty() || desc.empty()))
            return *this;

        _modifys.push_back(std::make_tuple(field, desc));

        _type = CHANGE_MODIFY;
        return *this;
    }

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &Modify(const std::vector<std::tuple<LibString, LibString>> &contents)
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


    SqlBuilder<SqlBuilderType::ALTER_TABLE> &Drop(const LibString &field)
    {
        if(UNLIKELY(field.empty()))
            return *this;

        _drops.push_back(field);
        _type = CHANGE_DROP;
        return *this;
    }

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &Drop(const std::vector<LibString> &fields)
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

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &AddIndex(const LibString &indexName, const std::vector<LibString> &fields, const LibString &usingDesc, const LibString &comment, bool usingFulltext = false, const LibString &fulltextWithParser = "")
    {
        if(UNLIKELY(indexName.empty() || fields.empty()))
            return *this;

        const auto &content = std::make_tuple(indexName, fields, usingDesc, comment, usingFulltext, fulltextWithParser);
        _addIndexs.push_back(content);
        _type = CHANGE_ADD_INDEX;
        return *this;
    }

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &AddIndex(const std::vector<std::tuple<LibString, std::vector<LibString>, LibString, LibString, bool, LibString>> &contents)
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

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &AddUniqueIndex(const LibString &indexName, const std::vector<LibString> &fields, const LibString &usingDesc, const LibString &comment)
    {
        if(UNLIKELY(indexName.empty() || fields.empty()))
            return *this;

        const auto &content = std::make_tuple(indexName, fields, usingDesc, comment);
        _addUniqueIndexs.push_back(content);
        _type = CHANGE_ADD_UNIQUE_INDEX;
        return *this;
    }

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &AddUniqueIndex(const std::vector<std::tuple<LibString, std::vector<LibString>, LibString, LibString>> &contents)
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

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &DropIndex(const LibString &indexName)
    {
        if(UNLIKELY(indexName.empty()))
            return *this;

        _dropIndexs.push_back(indexName);
        _type = CHANGE_DROP_INDEX;
        return *this;
    }

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &DropIndex(const std::vector<LibString> &indexNames)
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

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &AddPrimaryKey(const LibString &field)
    {
        if(UNLIKELY(field.empty()))
            return *this;
        
        _addPrimaryKeys.push_back(field);
        _type = CHANGE_ADD_PRIMARY_KEY;
        return *this;
    }

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &AddPrimaryKey(const std::vector<LibString> &fields)
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

    SqlBuilder<SqlBuilderType::ALTER_TABLE> &DropPrimaryKey()
    {
        _type = CHANGE_DROP_PRIMARY_KEY;
        return *this;
    }

    LibString ToSql() const
    {
        if((_type == CHANGE_UNKNOWN) || 
            (_table.empty()) )
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
        default:
            break;
        }

        return "";
    }

private:
    LibString _BuildAddSql() const
    {
        if(UNLIKELY(_adds.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("ALTER TABLE `%s`", _table.c_str());
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
        sql.AppendFormat("ALTER TABLE `%s`", _table.c_str());
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
        sql.AppendFormat("ALTER TABLE `%s`", _table.c_str());
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
        sql.AppendFormat("ALTER TABLE `%s`", _table.c_str());
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
        sql.AppendFormat("ALTER TABLE `%s`", _table.c_str());
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
        sql.AppendFormat("ALTER TABLE `%s`", _table.c_str());
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
        sql.AppendFormat("ALTER TABLE `%s`", _table.c_str());
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
        sql.AppendFormat("ALTER TABLE `%s` ADD PRIMARY KEY (%s)", _table.c_str(), StringUtil::ToString(_addPrimaryKeys, ",").c_str());
        return sql;
    }

    LibString _BuildDropPrimaryKeySql() const
    {
        LibString sql;
        sql.AppendFormat("ALTER TABLE `%s` DROP PRIMARY KEY", _table.c_str());
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
    };

    std::vector<std::tuple<LibString, LibString, LibString>> _adds; // field, desc, after
    std::vector<std::tuple<LibString, LibString>> _renames;         // oldField, newField
    std::vector<std::tuple<LibString, LibString>> _modifys;         // field, desc
    std::vector<LibString> _drops;                                  // field
    std::vector<std::tuple<LibString, std::vector<LibString>, LibString, LibString, bool, LibString>> _addIndexs; // index name, column name list, using btree等, comment, 是否使用FULLTEXT, fulltext with parser(建立全文索引并使用解析器,建议ngram)     
    std::vector<std::tuple<LibString, std::vector<LibString>, LibString, LibString>> _addUniqueIndexs; // index name, column name list, using btree等, comment
    std::vector<LibString> _dropIndexs;
    std::vector<LibString> _addPrimaryKeys;
    Int32 _type = CHANGE_UNKNOWN;
    LibString _table;
};

// ALTER TABLE `xxx` ADD/DROP/MODIFY COLUMN ...
template<>
class SqlBuilder<SqlBuilderType::SHOW_INDEX>
{
public:
    SqlBuilder() {}
    ~SqlBuilder() {}

    void Clear()
    {
        _table.clear();
    }

    SqlBuilder<SqlBuilderType::SHOW_INDEX> &Table(const LibString &table)
    {
        _table = table;
        return *this;
    }

    LibString ToSql() const
    {
        if(UNLIKELY(_table.empty()))
            return "";

        LibString sql;
        sql.AppendFormat("SHOW INDEX FROM `%s`", _table.c_str());
        return sql;
    }

private:
    LibString _table;
};

KERNEL_END

#endif

