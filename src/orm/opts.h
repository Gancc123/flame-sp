#ifndef FLAME_ORM_OPTS_H
#define FLAME_ORM_OPTS_H

#include "stmt.h"
#include "value.h"

#include <initializer_list>
#include <list>
namespace flame {
namespace orm {

/**
 * Set/Assignment
 */
inline AssignStmt set_(const ColumnStmt& col, const ValueStmt& val) {
    return AssignStmt(string_concat({col.to_str(), " = ", val.to_str()}));
}

inline AssignStmt set_(const ColumnStmt& col, const ColumnStmt& src) {
    return AssignStmt(string_concat({col.to_str(), " = ", src.to_str()}));
}

inline AssignStmt set_(const ColumnStmt& col, const ExprStmt& expr) {
    return AssignStmt(string_concat({col.to_str(), " = ", expr.to_str()}));
}

inline AssignStmt assign_(const ColumnStmt& col, const ValueStmt& val) {
    return set_(col, val);
}

inline AssignStmt assign_(const ColumnStmt& col, const ColumnStmt& src) {
    return set_(col, src);
}

inline AssignStmt assign_(const ColumnStmt& col, const ExprStmt& expr) {
    return set_(col, expr);
}

/**
 * Boolean Operator
 */
inline CondStmt not_(const CondStmt& cond) {
    return CondStmt(string_concat({"NOT (", cond.to_str(), ")"}));
}

inline CondStmt exists_(const SelectStmt& stmt) {
    return CondStmt(string_concat({"EXISTS (", stmt.to_str(), ")"}));
}

inline CondStmt not_exists_(const SelectStmt& stmt) {
    return CondStmt(string_concat({"NOT EXISTS (", stmt.to_str(), ")"}));
}

inline CondStmt and_(const CondStmt& lhs, const CondStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") AND (", rhs.to_str(), ")"}));
}

inline CondStmt or_(const CondStmt& lhs, const CondStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") OR (", rhs.to_str(), ")"}));
}

inline CondStmt between_(const ColumnStmt& col, const ExprStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(
        string_concat({col.to_str(), " BETWEEN ", lhs.to_str(), " AND ", rhs.to_str()})
    );
}

inline CondStmt between_(const ColumnStmt& col, const ValueStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(
        string_concat({col.to_str(), " BETWEEN ", lhs.to_str(), " AND ", rhs.to_str()})
    );
}

inline CondStmt between_(const ColumnStmt& col, const ExprStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(
        string_concat({col.to_str(), " BETWEEN ", lhs.to_str(), " AND ", rhs.to_str()})
    );
}

inline CondStmt between_(const ColumnStmt& col, const ValueStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(
        string_concat({col.to_str(), " BETWEEN ", lhs.to_str(), " AND ", rhs.to_str()})
    );
}

inline CondStmt not_between_(const ColumnStmt& col, const ExprStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(
        string_concat({col.to_str(), " NOT BETWEEN ", lhs.to_str(), " AND ", rhs.to_str()})
    );
}

inline CondStmt not_between_(const ColumnStmt& col, const ValueStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(
        string_concat({col.to_str(), " NOT BETWEEN ", lhs.to_str(), " AND ", rhs.to_str()})
    );
}

inline CondStmt not_between_(const ColumnStmt& col, const ExprStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(
        string_concat({col.to_str(), " NOT BETWEEN ", lhs.to_str(), " AND ", rhs.to_str()})
    );
}

inline CondStmt not_between_(const ColumnStmt& col, const ValueStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(
        string_concat({col.to_str(), " NOT BETWEEN ", lhs.to_str(), " AND ", rhs.to_str()})
    );
}

inline CondStmt like_(const ColumnStmt& col, const StringVal& pattern) {
    return CondStmt(string_concat({col.to_str(), " LIKE ", pattern.to_str()}));
}

inline CondStmt not_like_(const ColumnStmt& col, const StringVal& pattern) {
    return CondStmt(string_concat({col.to_str(), " NOT LIKE ", pattern.to_str()}));
}

inline CondStmt glob_(const ColumnStmt& col, const StringVal& pattern) {
    return CondStmt(string_concat({col.to_str(), " GLOB ", pattern.to_str()}));
}

inline CondStmt not_glob_(const ColumnStmt& col, const StringVal& pattern) {
    return CondStmt(string_concat({col.to_str(), " NOT GLOB ", pattern.to_str()}));
}

inline CondStmt is_null_(const ColumnStmt& col) {
    return CondStmt(string_concat({col.to_str(), " IS NULL"}));
}

inline CondStmt not_null_(const ColumnStmt& col) {
    return CondStmt(string_concat({col.to_str(), " IS NOT NULL"}));
}

template<typename T>
inline CondStmt in_(const ColumnStmt& col, const std::list<T>& vals) {
    std::string stmt = string_concat({col.to_str(), " IN ("});
    for (auto it = vals.begin(); it != vals.end(); it++) {
        if (it == vals.begin()) stmt += convert2string(*it);
        else string_append(stmt, ", ", convert2string(*it));
    }
    stmt += ")";
    return CondStmt(stmt);
}

template<>
inline CondStmt in_<ValueStmt>(const ColumnStmt& col, const std::list<ValueStmt>& vals) {
    std::string stmt = string_concat({col.to_str(), " IN ("});
    for (auto it = vals.begin(); it != vals.end(); it++) {
        if (it == vals.begin()) stmt += it->to_str();
        else string_append(stmt, ", ", it->to_str());
    }
    stmt += ")";
    return CondStmt(stmt);
}

/**
 * Stmt == Stmt
 */
inline CondStmt operator == (const ColumnStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " = ", rhs.to_str()}));
}

inline CondStmt operator == (const ValueStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " = ", rhs.to_str()}));
}

inline CondStmt operator == (const ExprStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") = (", rhs.to_str(), ")"}));
}

inline CondStmt operator == (const ColumnStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " = ", rhs.to_str()}));
}

inline CondStmt operator == (const ValueStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " = ", rhs.to_str()}));
}

inline CondStmt operator == (const ColumnStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " = (", rhs.to_str(), ")"}));
}

inline CondStmt operator == (const ExprStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") = ", rhs.to_str()}));
}

inline CondStmt operator == (const ValueStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " = (", rhs.to_str(), ")"}));
}

inline CondStmt operator == (const ExprStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") = ", rhs.to_str()}));
}

/**
 * Stmt == Stmt
 */
inline CondStmt operator != (const ColumnStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " <> ", rhs.to_str()}));
}

inline CondStmt operator != (const ValueStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " <> ", rhs.to_str()}));
}

inline CondStmt operator != (const ExprStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") <> (", rhs.to_str(), ")"}));
}

inline CondStmt operator != (const ColumnStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " <> ", rhs.to_str()}));
}

inline CondStmt operator != (const ValueStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " <> ", rhs.to_str()}));
}

inline CondStmt operator != (const ColumnStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " <> (", rhs.to_str(), ")"}));
}

inline CondStmt operator != (const ExprStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") <> ", rhs.to_str()}));
}

inline CondStmt operator != (const ValueStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " <> (", rhs.to_str(), ")"}));
}

inline CondStmt operator != (const ExprStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") <> ", rhs.to_str()}));
}

/**
 * Stmt > Stmt
 */
inline CondStmt operator > (const ColumnStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " > ", rhs.to_str()}));
}

inline CondStmt operator > (const ValueStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " > ", rhs.to_str()}));
}

inline CondStmt operator > (const ExprStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") > (", rhs.to_str(), ")"}));
}

inline CondStmt operator > (const ColumnStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " > ", rhs.to_str()}));
}

inline CondStmt operator > (const ValueStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " > ", rhs.to_str()}));
}

inline CondStmt operator > (const ColumnStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " > (", rhs.to_str(), ")"}));
}

inline CondStmt operator > (const ExprStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") > ", rhs.to_str()}));
}

inline CondStmt operator > (const ValueStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " > (", rhs.to_str(), ")"}));
}

inline CondStmt operator > (const ExprStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") > ", rhs.to_str()}));
}

/**
 * Stmt >= Stmt
 */
inline CondStmt operator >= (const ColumnStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " >= ", rhs.to_str()}));
}

inline CondStmt operator >= (const ValueStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " >= ", rhs.to_str()}));
}

inline CondStmt operator >= (const ExprStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") >= (", rhs.to_str(), ")"}));
}

inline CondStmt operator >= (const ColumnStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " >= ", rhs.to_str()}));
}

inline CondStmt operator >= (const ValueStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " >= ", rhs.to_str()}));
}

inline CondStmt operator >= (const ColumnStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " >= (", rhs.to_str(), ")"}));
}

inline CondStmt operator >= (const ExprStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") >= ", rhs.to_str()}));
}

inline CondStmt operator >= (const ValueStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " >= (", rhs.to_str(), ")"}));
}

inline CondStmt operator >= (const ExprStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") >= ", rhs.to_str()}));
}

/**
 * Stmt < Stmt
 */
inline CondStmt operator < (const ColumnStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " < ", rhs.to_str()}));
}

inline CondStmt operator < (const ValueStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " < ", rhs.to_str()}));
}

inline CondStmt operator < (const ExprStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") < (", rhs.to_str(), ")"}));
}

inline CondStmt operator < (const ColumnStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " < ", rhs.to_str()}));
}

inline CondStmt operator < (const ValueStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " < ", rhs.to_str()}));
}

inline CondStmt operator < (const ColumnStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " < (", rhs.to_str(), ")"}));
}

inline CondStmt operator < (const ExprStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") < ", rhs.to_str()}));
}

inline CondStmt operator < (const ValueStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " < (", rhs.to_str(), ")"}));
}

inline CondStmt operator < (const ExprStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") < ", rhs.to_str()}));
}

/**
 * Stmt <= Stmt
 */
inline CondStmt operator <= (const ColumnStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " <= ", rhs.to_str()}));
}

inline CondStmt operator <= (const ValueStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " <= ", rhs.to_str()}));
}

inline CondStmt operator <= (const ExprStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") <= (", rhs.to_str(), ")"}));
}

inline CondStmt operator <= (const ColumnStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " <= ", rhs.to_str()}));
}

inline CondStmt operator <= (const ValueStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " <= ", rhs.to_str()}));
}

inline CondStmt operator <= (const ColumnStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " <= (", rhs.to_str(), ")"}));
}

inline CondStmt operator <= (const ExprStmt& lhs, const ColumnStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") <= ", rhs.to_str()}));
}

inline CondStmt operator <= (const ValueStmt& lhs, const ExprStmt& rhs) {
    return CondStmt(string_concat({lhs.to_str(), " <= (", rhs.to_str(), ")"}));
}

inline CondStmt operator <= (const ExprStmt& lhs, const ValueStmt& rhs) {
    return CondStmt(string_concat({"(", lhs.to_str(), ") <= ", rhs.to_str()}));
}

/**
 * ExprStmt + ExprStmt
 */
inline ExprStmt operator + (const ExprStmt& lhs, const ExprStmt& rhs) {
    return ExprStmt(string_concat({"(", lhs.to_str(), ") + (", rhs.to_str(), ")"}));
}

inline ExprStmt operator + (const ExprStmt& lhs, const ColumnStmt& rhs) {
    return ExprStmt(string_concat({"(", lhs.to_str(), ") + ", rhs.to_str()}));
}

inline ExprStmt operator + (const ColumnStmt& lhs, const ExprStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " + (", rhs.to_str(), ")"}));
}

inline ExprStmt operator + (const ColumnStmt& lhs, const ColumnStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " + ", rhs.to_str()}));
}

inline ExprStmt operator + (const ColumnStmt& lhs, const ValueStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " + ", rhs.to_str()}));
}

inline ExprStmt operator + (const ValueStmt& lhs, const ColumnStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " + ", rhs.to_str()}));
}

inline ExprStmt operator + (const ExprStmt& lhs, const ValueStmt& rhs) {
    return ExprStmt(string_concat({"(", lhs.to_str(), ") + ", rhs.to_str()}));
}

inline ExprStmt operator + (const ValueStmt& lhs, const ExprStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " + (", rhs.to_str(), ")"}));
}

/**
 * ExprStmt - ExprStmt
 */
inline ExprStmt operator - (const ExprStmt& lhs, const ExprStmt& rhs) {
    return ExprStmt(string_concat({"(", lhs.to_str(), ") - (", rhs.to_str(), ")"}));
}

inline ExprStmt operator - (const ExprStmt& lhs, const ColumnStmt& rhs) {
    return ExprStmt(string_concat({"(", lhs.to_str(), ") - ", rhs.to_str()}));
}

inline ExprStmt operator - (const ColumnStmt& lhs, const ExprStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " - (", rhs.to_str(), ")"}));
}

inline ExprStmt operator - (const ColumnStmt& lhs, const ColumnStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " - ", rhs.to_str()}));
}

inline ExprStmt operator - (const ColumnStmt& lhs, const ValueStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " - ", rhs.to_str()}));
}

inline ExprStmt operator - (const ValueStmt& lhs, const ColumnStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " - ", rhs.to_str()}));
}

inline ExprStmt operator - (const ExprStmt& lhs, const ValueStmt& rhs) {
    return ExprStmt(string_concat({"(", lhs.to_str(), ") - ", rhs.to_str()}));
}

inline ExprStmt operator - (const ValueStmt& lhs, const ExprStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " - (", rhs.to_str(), ")"}));
}

/**
 * ExprStmt * ExprStmt
 */
inline ExprStmt operator * (const ExprStmt& lhs, const ExprStmt& rhs) {
    return ExprStmt(string_concat({"(", lhs.to_str(), ") * (", rhs.to_str(), ")"}));
}

inline ExprStmt operator * (const ExprStmt& lhs, const ColumnStmt& rhs) {
    return ExprStmt(string_concat({"(", lhs.to_str(), ") * ", rhs.to_str()}));
}

inline ExprStmt operator * (const ColumnStmt& lhs, const ExprStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " * (", rhs.to_str(), ")"}));
}

inline ExprStmt operator * (const ColumnStmt& lhs, const ColumnStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " * ", rhs.to_str()}));
}

inline ExprStmt operator * (const ColumnStmt& lhs, const ValueStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " * ", rhs.to_str()}));
}

inline ExprStmt operator * (const ValueStmt& lhs, const ColumnStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " * ", rhs.to_str()}));
}

inline ExprStmt operator * (const ExprStmt& lhs, const ValueStmt& rhs) {
    return ExprStmt(string_concat({"(", lhs.to_str(), ") * ", rhs.to_str()}));
}

inline ExprStmt operator * (const ValueStmt& lhs, const ExprStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " * (", rhs.to_str(), ")"}));
}

/**
 * ExprStmt / ExprStmt
 */
inline ExprStmt operator / (const ExprStmt& lhs, const ExprStmt& rhs) {
    return ExprStmt(string_concat({"(", lhs.to_str(), ") / (", rhs.to_str(), ")"}));
}

inline ExprStmt operator / (const ExprStmt& lhs, const ColumnStmt& rhs) {
    return ExprStmt(string_concat({"(", lhs.to_str(), ") / ", rhs.to_str()}));
}

inline ExprStmt operator / (const ColumnStmt& lhs, const ExprStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " / (", rhs.to_str(), ")"}));
}

inline ExprStmt operator / (const ColumnStmt& lhs, const ColumnStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " / ", rhs.to_str()}));
}

inline ExprStmt operator / (const ColumnStmt& lhs, const ValueStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " / ", rhs.to_str()}));
}

inline ExprStmt operator / (const ValueStmt& lhs, const ColumnStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " / ", rhs.to_str()}));
}

inline ExprStmt operator / (const ExprStmt& lhs, const ValueStmt& rhs) {
    return ExprStmt(string_concat({"(", lhs.to_str(), ") / ", rhs.to_str()}));
}

inline ExprStmt operator / (const ValueStmt& lhs, const ExprStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " / (", rhs.to_str(), ")"}));
}

/**
 * ExprStmt % ExprStmt
 */
inline ExprStmt operator % (const ExprStmt& lhs, const ExprStmt& rhs) {
    return ExprStmt(string_concat({"(", lhs.to_str(), ") * (", rhs.to_str(), ")"}));
}

inline ExprStmt operator % (const ExprStmt& lhs, const ColumnStmt& rhs) {
    return ExprStmt(string_concat({"(", lhs.to_str(), ") * ", rhs.to_str()}));
}

inline ExprStmt operator % (const ColumnStmt& lhs, const ExprStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " * (", rhs.to_str(), ")"}));
}

inline ExprStmt operator % (const ColumnStmt& lhs, const ColumnStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " * ", rhs.to_str()}));
}

inline ExprStmt operator % (const ColumnStmt& lhs, const ValueStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " * ", rhs.to_str()}));
}

inline ExprStmt operator % (const ValueStmt& lhs, const ColumnStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " * ", rhs.to_str()}));
}

inline ExprStmt operator % (const ExprStmt& lhs, const ValueStmt& rhs) {
    return ExprStmt(string_concat({"(", lhs.to_str(), ") * ", rhs.to_str()}));
}

inline ExprStmt operator % (const ValueStmt& lhs, const ExprStmt& rhs) {
    return ExprStmt(string_concat({lhs.to_str(), " * (", rhs.to_str(), ")"}));
}

} // namespace orm
} // namespace flame

#endif // FLAME_ORM_OPTS_h