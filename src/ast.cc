#include <cassert>
#include <limits>
#include <ctime>
#include <type_traits>

#include "ast.hh"

namespace ast {

/* FIXME: implement actual integer promotions etc */

static c_value eval_unary(c_expr const &e) {
    c_value baseval = e.un.expr->eval();
    switch (e.un.op) {
        case c_expr_unop::UNP:
            break;
        case c_expr_unop::UNM:
            baseval.i = -baseval.i;
            break;
        case c_expr_unop::NOT:
            baseval.i = !baseval.i;
            break;
        case c_expr_unop::BNOT:
            baseval.i = ~baseval.i;
            break;
        default:
            assert(false);
            break;
    }
    return baseval;
}

static c_value eval_binary(c_expr const &e) {
    c_value lval = e.bin.lhs->eval();
    c_value rval = e.bin.rhs->eval();
    c_value ret;
    switch (e.bin.op) {
        case c_expr_binop::ADD:
            ret.i = lval.i + rval.i; break;
        case c_expr_binop::SUB:
            ret.i = lval.i - rval.i; break;
        case c_expr_binop::MUL:
            ret.i = lval.i * rval.i; break;
        case c_expr_binop::DIV:
            ret.i = lval.i / rval.i; break;
        case c_expr_binop::MOD:
            ret.i = lval.i % rval.i; break;

        case c_expr_binop::EQ:
            ret.i = (lval.i == rval.i); break;
        case c_expr_binop::NEQ:
            ret.i = (lval.i != rval.i); break;
        case c_expr_binop::GT:
            ret.i = (lval.i > rval.i); break;
        case c_expr_binop::LT:
            ret.i = (lval.i < rval.i); break;
        case c_expr_binop::GE:
            ret.i = (lval.i >= rval.i); break;
        case c_expr_binop::LE:
            ret.i = (lval.i <= rval.i); break;

        case c_expr_binop::AND:
            ret.i = lval.i && rval.i; break;
        case c_expr_binop::OR:
            ret.i = lval.i || rval.i; break;

        case c_expr_binop::BAND:
            ret.i = lval.i & rval.i; break;
        case c_expr_binop::BOR:
            ret.i = lval.i | rval.i; break;
        case c_expr_binop::BXOR:
            ret.i = lval.i ^ rval.i; break;
        case c_expr_binop::LSH:
            ret.i = lval.i << rval.i; break;
        case c_expr_binop::RSH:
            ret.i = lval.i >> rval.i; break;

        default:
            assert(false);
            break;
    }
    return ret;
}

static c_value eval_ternary(c_expr const &e) {
    c_value cval = e.tern.cond->eval();
    if (cval.i) {
        return e.tern.texpr->eval();
    }
    return e.tern.fexpr->eval();
}

c_value c_expr::eval() const {
    c_value ret;
    switch (type) {
        case c_expr_type::BINARY:
            return eval_binary(*this);
        case c_expr_type::UNARY:
            return eval_unary(*this);
        case c_expr_type::TERNARY:
            return eval_ternary(*this);
        case c_expr_type::INT:
            ret.i = val.i; break;
        case c_expr_type::UINT:
            ret.i = int(val.u); break;
        case c_expr_type::LONG:
            ret.i = int(val.l); break;
        case c_expr_type::ULONG:
            ret.i = int(val.ul); break;
        case c_expr_type::LLONG:
            ret.i = int(val.ll); break;
        case c_expr_type::ULLONG:
            ret.i = int(val.ull); break;
        case c_expr_type::FLOAT:
            ret.i = int(val.f); break;
        case c_expr_type::DOUBLE:
            ret.i = int(val.d); break;
        case c_expr_type::CHAR:
            ret.i = int(val.c); break;
        case c_expr_type::BOOL:
            ret.i = int(val.b); break;
        default:
            ret.i = 0; break;
    }
    return ret;
}

void c_param::do_serialize(std::string &o) const {
    p_type.do_serialize(o);
    if (!this->name.empty()) {
        if (o.back() != '*') {
            o += ' ';
        }
        o += this->name;
    }
}

void c_function::do_serialize_full(std::string &o, bool fptr, int cv) const {
    p_result.do_serialize(o);
    if (o.back() != '*') {
        o += ' ';
    }
    if (!fptr) {
        o += "()";
        return;
    }
    o += "(*";
    if (cv & C_CV_CONST) {
        o += " const";
    }
    if (cv & C_CV_VOLATILE) {
        o += " volatile";
    }
    o += ")()";
}

c_type::c_type(c_function tp, int qual, int cbt):
    c_object{}, p_fptr{new c_function{std::move(tp)}},
    p_type{cbt | uint32_t(qual)}
{}

c_type::~c_type() {
    if (!owns()) {
        return;
    }
    int tp = type();
    if ((tp == C_BUILTIN_FPTR) || (tp == C_BUILTIN_FUNC)) {
        delete p_fptr;
    } else if (tp == C_BUILTIN_PTR) {
        delete p_ptr;
    }
}

c_type::c_type(c_type const &v): c_object{v.name}, p_type{v.p_type} {
    bool weak = !owns();
    int tp = type();
    if ((tp == C_BUILTIN_FPTR) || (tp == C_BUILTIN_FUNC)) {
        p_fptr = weak ? v.p_fptr : new c_function{*v.p_fptr};
    } else if (tp == C_BUILTIN_PTR) {
        p_ptr = weak ? v.p_ptr : new c_type{*v.p_ptr};
    }
}

c_type::c_type(c_type &&v):
    c_object{std::move(v.name)}, p_ptr{std::exchange(v.p_ptr, nullptr)},
    p_type{v.p_type}
{}

void c_type::do_serialize(std::string &o) const {
    int tcv = cv();
    int ttp = type();
    switch (ttp) {
        case C_BUILTIN_PTR:
            p_ptr->do_serialize(o);
            if (o.back() != '*') {
                o += ' ';
            }
            o += '*';
            break;
        case C_BUILTIN_FPTR:
        case C_BUILTIN_FUNC:
            /* cv is handled by func serializer */
            p_fptr->do_serialize_full(o, (ttp == C_BUILTIN_FPTR), tcv);
            return;
        default:
            o += this->name;
            break;
    }
    if (tcv & C_CV_CONST) {
        o += " const";
    }
    if (tcv & C_CV_VOLATILE) {
        o += " volatile";
    }
}

#define C_BUILTIN_CASE(bt) case C_BUILTIN_##bt: \
    return ast::builtin_ffi_type<C_BUILTIN_##bt>();

ffi_type *c_type::libffi_type() const {
    switch (c_builtin(type())) {
        C_BUILTIN_CASE(VOID)
        C_BUILTIN_CASE(PTR)

        case C_BUILTIN_FPTR:
        case C_BUILTIN_FUNC:
            return p_fptr->libffi_type();

        case C_BUILTIN_STRUCT:
            return p_crec->libffi_type();
        case C_BUILTIN_ENUM:
            return p_cenum->libffi_type();

        C_BUILTIN_CASE(FLOAT)
        C_BUILTIN_CASE(DOUBLE)
        C_BUILTIN_CASE(LDOUBLE)

        C_BUILTIN_CASE(BOOL)

        C_BUILTIN_CASE(CHAR)
        C_BUILTIN_CASE(SCHAR)
        C_BUILTIN_CASE(UCHAR)
        C_BUILTIN_CASE(SHORT)
        C_BUILTIN_CASE(USHORT)
        C_BUILTIN_CASE(INT)
        C_BUILTIN_CASE(UINT)
        C_BUILTIN_CASE(LONG)
        C_BUILTIN_CASE(ULONG)
        C_BUILTIN_CASE(LLONG)
        C_BUILTIN_CASE(ULLONG)

        C_BUILTIN_CASE(WCHAR)
        C_BUILTIN_CASE(CHAR16)
        C_BUILTIN_CASE(CHAR32)

        C_BUILTIN_CASE(INT8)
        C_BUILTIN_CASE(INT16)
        C_BUILTIN_CASE(INT32)
        C_BUILTIN_CASE(INT64)
        C_BUILTIN_CASE(UINT8)
        C_BUILTIN_CASE(UINT16)
        C_BUILTIN_CASE(UINT32)
        C_BUILTIN_CASE(UINT64)

        C_BUILTIN_CASE(SIZE)
        C_BUILTIN_CASE(SSIZE)
        C_BUILTIN_CASE(INTPTR)
        C_BUILTIN_CASE(UINTPTR)
        C_BUILTIN_CASE(PTRDIFF)

        C_BUILTIN_CASE(TIME)

        case C_BUILTIN_INVALID:
            break;

        /* intentionally no default so that missing members are caught */
    }

    assert(false);
    return nullptr;
}

#undef C_BUILTIN_CASE

/* lua is not thread safe, so the FFI doesn't need to be either */

/* the list of declarations; actually stored */
static std::vector<std::unique_ptr<c_object>> decl_list;

/* mapping for quick lookups */
static std::unordered_map<std::string, c_object const *> decl_map;

void add_decl(c_object *decl) {
    if (decl_map.find(decl->name) != decl_map.end()) {
        throw redefine_error{decl->name};
    }

    decl_list.emplace_back(decl);
    auto &d = *decl_list.back();
    decl_map.emplace(d.name, &d);

    /* enums: register fields as constant values
     * FIXME: don't hardcode like this
     */
    if (d.obj_type() == c_object_type::ENUM) {
        for (auto &fld: d.as<c_enum>().fields()) {
            c_value val;
            val.i = fld.value;
            add_decl(
                new c_constant{fld.name, c_type{"int", C_BUILTIN_INT, 0}, val}
            );
        }
    }
}

c_object const *lookup_decl(std::string const &name) {
    auto it = decl_map.find(name);
    if (it == decl_map.end()) {
        return nullptr;
    }
    return it->second;
}

std::string request_name() {
    char buf[32];
    /* could do something better, this will do to avoid clashes for now... */
    snprintf(buf, sizeof(buf), "%zu", decl_list.size());
    return std::string{static_cast<char const *>(buf)};
}

} /* namespace ast */
