#ifndef FLAME_ORM_COLS_H
#define FLAME_ORM_COLS_H

#include "stmt.h"
#include "value.h"

namespace flame {
namespace orm {

class IntColumn : public Column {
public:
    virtual ~IntColumn() {}

protected:
    IntColumn(Table* parent, 
        const std::string& col_name,
        const std::string& type_name,
        uint32_t flags,
        const std::string& def,
        const std::string& extra = "")
        : Column(parent, col_name, type_name, flags, def, extra) 
    {
        // nothing
    }
}; // class IntColumn

class UIntColumn : public Column {
public:
    virtual ~UIntColumn() {}

protected:
    UIntColumn(Table* parent, 
        const std::string& col_name,
        const std::string& type_name,
        uint32_t flags,
        const std::string& def,
        const std::string& extra = "")
        : Column(parent, col_name, type_name, flags, def, extra) 
    {
        // nothing
    }
}; // class UIntColumn

class DecimalColumn : public Column {
public:
    virtual ~DecimalColumn() {}

protected:
    DecimalColumn(Table* parent, 
        const std::string& col_name,
        const std::string& type_name,
        uint32_t flags,
        const std::string& def,
        const std::string& extra = "")
        : Column(parent, col_name, type_name, flags, def, extra) 
    {
        // nothing
    }
}; // class DecimalColumn

class StringColumn : public Column {
public:
    virtual ~StringColumn() {}

protected:
    StringColumn(Table* parent, 
        const std::string& col_name,
        const std::string& type_name,
        uint32_t flags,
        const std::string& def,
        const std::string& extra = "")
        : Column(parent, col_name, type_name, flags, def, extra) 
    {
        // nothing
    }
}; // class StringColumn

/**
 * TINYINT
 */
class TinyIntCol : public IntColumn {
public:
    TinyIntCol(Table* parent, 
        const std::string& col_name,
        uint32_t flags,
        int8_t def,
        const std::string& extra = "")
        : IntColumn(parent, col_name, "TINYINT", flags, convert2string(def), extra) 
    {
        // nothing
    }

    TinyIntCol(Table* parent, const std::string& col_name, uint32_t flags)
        : IntColumn(parent, col_name, "TINYINT", flags, "", "") 
    {
        // nothing
    }

    TinyIntCol(Table* parent, const std::string& col_name)
        : IntColumn(parent, col_name, "TINYINT", ColFlag::NONE, "", "") 
    {
        // nothing
    }

    ~TinyIntCol() {}
}; // class TinyIntCol

/**
 * TINYINT UNSIGNED
 */
class UTinyIntCol : public UIntColumn {
public:
    UTinyIntCol(Table* parent, 
        const std::string& col_name,
        uint32_t flags,
        uint8_t def,
        const std::string& extra = "")
        : UIntColumn(parent, col_name, "TINYINT UNSIGNED", flags, convert2string(def), extra) 
    {
        // nothing
    }

    UTinyIntCol(Table* parent, const std::string& col_name, uint32_t flags)
        : UIntColumn(parent, col_name, "TINYINT UNSIGNED", flags, "", "") 
    {
        // nothing
    }

    UTinyIntCol(Table* parent, const std::string& col_name)
        : UIntColumn(parent, col_name, "TINYINT UNSIGNED", ColFlag::NONE, "", "") 
    {
        // nothing
    }

    ~UTinyIntCol() {}
}; // class UTinyIntCol

/**
 * SMALLINT
 */
class SmallIntCol : public IntColumn {
public:
    SmallIntCol(Table* parent, 
        const std::string& col_name,
        uint32_t flags,
        int16_t def,
        const std::string& extra = "")
        : IntColumn(parent, col_name, "SMALLINT", flags, convert2string(def), extra) 
    {
        // nothing
    }

    SmallIntCol(Table* parent, const std::string& col_name, uint32_t flags)
        : IntColumn(parent, col_name, "SMALLINT", flags, "", "") 
    {
        // nothing
    }

    SmallIntCol(Table* parent, const std::string& col_name)
        : IntColumn(parent, col_name, "SMALLINT", ColFlag::NONE, "", "") 
    {
        // nothing
    }

    ~SmallIntCol() {}
}; // class SmallIntCol

/**
 * SAMLLINT UNSIGNED
 */
class USmallIntCol : public UIntColumn {
public:
    USmallIntCol(Table* parent, 
        const std::string& col_name,
        uint32_t flags, 
        uint16_t def,
        const std::string& extra = "")
        : UIntColumn(parent, col_name, "SMALLINT UNSIGNED", flags, convert2string(def), extra) 
    {
        // nothing
    }

    USmallIntCol(Table* parent, const std::string& col_name, uint32_t flags)
        : UIntColumn(parent, col_name, "SMALLINT UNSIGNED", flags, "", "") 
    {
        // nothing
    }

    USmallIntCol(Table* parent, const std::string& col_name)
        : UIntColumn(parent, col_name, "SMALLINT UNSIGNED", ColFlag::NONE, "", "") 
    {
        // nothing
    }

    ~USmallIntCol() {}
}; // class USmallIntCol

/**
 * INT
 */
class IntCol : public IntColumn {
public:
    IntCol(Table* parent, 
        const std::string& col_name,
        uint32_t flags, 
        int32_t def,
        const std::string& extra = "")
        : IntColumn(parent, col_name, "TINYINT", flags, convert2string(def), extra) 
    {
        // nothing
    }

    IntCol(Table* parent, const std::string& col_name, uint32_t flags)
        : IntColumn(parent, col_name, "TINYINT", flags, "", "") 
    {
        // nothing
    }

    IntCol(Table* parent, const std::string& col_name)
        : IntColumn(parent, col_name, "TINYINT", ColFlag::NONE, "", "") 
    {
        // nothing
    }

    ~IntCol() {}
}; // class IntCol

/**
 * INT UNSIGNED
 */
class UIntCol : public UIntColumn {
public:
    UIntCol(Table* parent, 
        const std::string& col_name,
        uint32_t flags, 
        uint32_t def,
        const std::string& extra = "")
        : UIntColumn(parent, col_name, "INT", flags, convert2string(def), extra) 
    {
        // nothing
    }

    UIntCol(Table* parent, const std::string& col_name, uint32_t flags)
        : UIntColumn(parent, col_name, "INT", flags, "", "") 
    {
        // nothing
    }

    UIntCol(Table* parent, const std::string& col_name)
        : UIntColumn(parent, col_name, "INT", ColFlag::NONE, "", "") 
    {
        // nothing
    }

    ~UIntCol() {}
}; // class TinyIntCol

/**
 * BIGINT
 */
class BigIntCol : public IntColumn {
public:
    BigIntCol(Table* parent, 
        const std::string& col_name,
        uint32_t flags,
        int64_t def,
        const std::string& extra = "")
        : IntColumn(parent, col_name, "BIGINT", flags, convert2string(def), extra) 
    {
        // nothing
    }

    BigIntCol(Table* parent, const std::string& col_name, uint32_t flags)
        : IntColumn(parent, col_name, "BIGINT", flags, "", "") 
    {
        // nothing
    }

    BigIntCol(Table* parent, const std::string& col_name)
        : IntColumn(parent, col_name, "BIGINT", ColFlag::NONE, "", "") 
    {
        // nothing
    }

    ~BigIntCol() {}
}; // class BigIntCol

/**
 * BIGINT UNSIGNED
 */
class UBigIntCol : public UIntColumn {
public:
    UBigIntCol(Table* parent, 
        const std::string& col_name,
        uint32_t flags, 
        uint64_t def,
        const std::string& extra = "")
        : UIntColumn(parent, col_name, "BIGINT UNSIGNED", flags, convert2string(def), extra) 
    {
        // nothing
    }

    UBigIntCol(Table* parent, const std::string& col_name, uint32_t flags)
        : UIntColumn(parent, col_name, "BIGINT UNSIGNED", flags, "", "") 
    {
        // nothing
    }

    UBigIntCol(Table* parent, const std::string& col_name)
        : UIntColumn(parent, col_name, "BIGINT UNSIGNED", ColFlag::NONE, "", "") 
    {
        // nothing
    }

    ~UBigIntCol() {}
}; // class TinyIntCol

/**
 * DOUBLE
 */
class DoubleCol : public DecimalColumn {
public:
    DoubleCol(Table* parent, 
        const std::string& col_name,
        uint32_t flags,
        double def,
        const std::string& extra = "")
        : DecimalColumn(parent, col_name, "DOUBLE", flags, convert2string(def), extra) 
    {
        // nothing
    }

    DoubleCol(Table* parent, const std::string& col_name, uint32_t flags)
        : DecimalColumn(parent, col_name, "DOUBLE", flags, "", "") 
    {
        // nothing
    }

    DoubleCol(Table* parent, const std::string& col_name)
        : DecimalColumn(parent, col_name, "DOUBLE", ColFlag::NONE, "", "") 
    {
        // nothing
    }

    ~DoubleCol() {}
}; // class DoubleCol

/**
 * STRING
 */
class StringCol : public StringColumn {
public:
    StringCol(Table* parent, 
        const std::string& col_name,
        uint32_t len,
        uint32_t flags,
        const StringVal& def,
        const std::string& extra = "")
        : StringColumn(parent, col_name, 
            string_concat({"VARCHAR(", convert2string(len), ")"}), 
            flags, def.empty() ? "" : def.to_str(), extra) 
    {
        // nothing
    }

    StringCol(Table* parent, const std::string& col_name, uint32_t len, uint32_t flags)
        : StringCol(parent, col_name, len, flags, "", "") 
    {
        // nothing
    }

    StringCol(Table* parent, const std::string& col_name, uint32_t len)
        : StringCol(parent, col_name, len, ColFlag::NONE, "", "")
    {
        // nothing
    }

    ~StringCol() {}
}; // class DoubleCol

} // namespace orm
} // namespace flame

#endif // FLAME_ORM_COLS_H