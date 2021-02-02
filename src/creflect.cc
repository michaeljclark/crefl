#include <cstdio>
#include <cstdarg>
#include <cinttypes>

#include <vector>

#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ParentMapContext.h"
#include <clang/Frontend/FrontendPluginRegistry.h>

#include "cmodel.h"
#include "creflect.h"

using namespace clang;

static clang::FrontendPluginRegistry::Add<crefl::CReflectAction>
    X("crefl", "emit reflection metadata.");

static void log_debug(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    std::vector<char> buf(256);
    int len = vsnprintf(buf.data(), buf.size(), fmt, args);
    fwrite(buf.data(), 1, len, stderr);
    va_end(args);
}

#define debugf(...) if(debug) log_debug(__VA_ARGS__)

static const char* tagKindString(TagTypeKind k) {
    switch (k) {
    case TTK_Struct: return "struct";
    case TTK_Interface: return "interface";
    case TTK_Union: return "union";
    case TTK_Class: return "class";
    case TTK_Enum: return "enum";
    default: return "unknown";
    }
}

struct CReflectVisitor : public RecursiveASTVisitor<CReflectVisitor>
{
    ASTContext &context;
    decl_db *db;
    decl_ref last;
    llvm::SmallVector<decl_ref,16> stack;
    llvm::SmallMapVector<int64_t,_Id, 16> idmap;
    bool debug;

    CReflectVisitor(ASTContext &context, decl_db *db)
        : context(context), db(db), last(), debug(true) {}

    std::string decl_path(Decl *d)
    {
        std::string s;
        std::vector<std::string> dl;
        dl.push_back(d->getDeclKindName());
        DeclContext *dc = d->getLexicalDeclContext();
        while (dc) {
            dl.push_back(dc->getDeclKindName());
            dc = dc->getLexicalParent();
        }
        for (auto di = dl.begin(); di != dl.end(); di++) {
            if (s.size() > 0) s.append(".");
            s.append(*di);
        }
        return s;
    }

    void print_decl(Decl *d)
    {
        std::string dps = decl_path(d);
        SourceLocation sl = d->getLocation();
        std::string sls = sl.printToString(context.getSourceManager());
        Decl *nd = d->getNextDeclInContext();
        if (nd) {
            log_debug("(%" PRId64 " -> %" PRId64 ") %s : %s\n", d->getID(), nd->getID(), dps.c_str(), sls.c_str());
        } else {
            log_debug("(%" PRId64 ") %s : %s\n", d->getID(), dps.c_str(), sls.c_str());
        }
    }

    decl_ref get_intrinsic_type(QualType q)
    {
        TypeInfo t = context.getTypeInfo(q);

        bool _is_scalar = q->isScalarType();
        bool _is_complex = q->isComplexType();
        bool _is_vector = q->isVectorType();
        bool _is_array = q->isArrayType();
        bool _is_union = q->isUnionType();
        bool _is_struct = q->isStructureOrClassType();
        bool _is_const = (q.getLocalFastQualifiers() & Qualifiers::Const);
        bool _is_restrict = (q.getLocalFastQualifiers() & Qualifiers::Restrict);
        bool _is_volatile = (q.getLocalFastQualifiers() & Qualifiers::Volatile);

        // TODO - arrays, attributes, const, volatile, restrict, unaligned

        decl_ref tr = decl_ref { db, 0 };

        if (_is_scalar) {
            auto stk = q->getScalarTypeKind();
            switch (stk) {
            case clang::Type::ScalarTypeKind::STK_CPointer: {
                 tr = decl_find_intrinsic(db, _top | _void, t.Width);
                 break;
            }
            case clang::Type::ScalarTypeKind::STK_BlockPointer:
                break;
            case clang::Type::ScalarTypeKind::STK_ObjCObjectPointer:
                break;
            case clang::Type::ScalarTypeKind::STK_MemberPointer:
                break;
            case clang::Type::ScalarTypeKind::STK_Bool: {
                tr = decl_find_intrinsic(db, _top | _sint, 1);
                break;
            }
            case clang::Type::ScalarTypeKind::STK_Integral: {
                bool _is_unsigned = q->isUnsignedIntegerType();
                tr = decl_find_intrinsic(db, _top |
                    (_is_unsigned ? _uint : _sint), t.Width);
                break;
            }
            case clang::Type::ScalarTypeKind::STK_Floating: {
                tr = decl_find_intrinsic(db, _top | _float, t.Width);
                break;
            }
            case clang::Type::ScalarTypeKind::STK_IntegralComplex:
                break;
            case clang::Type::ScalarTypeKind::STK_FloatingComplex:
                break;
            case clang::Type::ScalarTypeKind::STK_FixedPoint:
                break;
            }
        }
        else if (_is_struct) {
            const RecordType *rt = q->getAsStructureType();
            const RecordDecl *rd = rt->getDecl();
            tr = decl_ref { db, idmap[rd->getID()] };
        }
        else if (_is_union) {
            const RecordType *rt = q->getAsUnionType();
            const RecordDecl *rd = rt->getDecl();
            tr = decl_ref { db, idmap[rd->getID()] };
        }
        else if (_is_array) {
            /* also {Incomplete,DependentSized,Variable}ArrayType */
            const ConstantArrayType * cat = context.getAsConstantArrayType(q);
            const ArrayType *at = context.getAsArrayType(q);
            const QualType q = at->getElementType();

            char namebuf[80];

            decl_ref ti = get_intrinsic_type(q);

            tr = decl_new(db, _decl_array, _top);
            decl_ptr(tr)->_decl_array._decl = decl_ref_idx(ti);
            if (cat) {
                u64 _size = cat->getSize().getLimitedValue();
                decl_ptr(tr)->_decl_array._size = _size;
                snprintf(namebuf, sizeof(namebuf), "%s[%llu]", decl_name(ti), _size);
            } else {
                snprintf(namebuf, sizeof(namebuf), "%s[]", decl_name(ti));
            }
            decl_name_new(tr, namebuf);
        }
        else if (_is_complex) {
            const ComplexType *ct = q->getAs<ComplexType>();
            // TODO
        }
        else if (_is_vector) {
            const VectorType *vt = q->getAs<VectorType>();
            // TODO
        }
        else {
            // TODO
        }

        debugf("\tscalar:%d complex:%d vector:%d array:%d struct:%d"
               " union:%d const:%d volatile:%d restrict:%d\n",
            _is_scalar, _is_complex, _is_vector, _is_array, _is_struct,
            _is_union, _is_const, _is_volatile, _is_restrict);

        return tr;
    }

    template <typename P> decl_ref get_parent(Decl *d)
    {
        const auto& parents = context.getParents(*d);
        if (parents.size() == 0) return decl_ref { db, 0 };
        const P *p = parents[0].get<P>();
        if (!p) return decl_ref { db, 0 };
        return decl_ref { db, idmap[p->getID()] };
    }

    bool VisitPragmaCommentDecl(PragmaCommentDecl *d)
    {
        /* PragmaCommentDecl -> Decl */

        if (d->isInvalidDecl()) return true;
        if (debug) print_decl(d);

        return true;
    }

    bool VisitTranslationUnitDecl(TranslationUnitDecl *d)
    {
        /* TranslationUnitDecl -> Decl */

        if (d->isInvalidDecl()) return true;
        if (debug) print_decl(d);

        return true;
    }

    bool VisitTypedefDecl(TypedefDecl *d)
    {
        /* TypedefDecl -> TypeNameDecl -> TypeDecl -> NamedDecl -> Decl */

        if (d->isInvalidDecl()) return true;
        if (debug) print_decl(d);

        const QualType q = d->getUnderlyingType();
        debugf("\tname:\"%s\" type:%s\n",
            d->clang::NamedDecl::getNameAsString().c_str(),
            q.getAsString().c_str());

        /* create typedef */
        decl_ref r = decl_new(db, _decl_typedef, _top);
        decl_name_new(r, d->clang::NamedDecl::getNameAsString().c_str());
        idmap[d->clang::Decl::getID()] = decl_ref_idx(r);
        decl_ptr(r)->_decl_typedef._decl = decl_ref_idx(get_intrinsic_type(q));

        /* create prev next link */
        if (decl_ref_idx(last) && !decl_ptr(last)->_next) {
            decl_ptr(last)->_next = decl_ref_idx(r);
        }

        /* create parent link */
        decl_ref parent = get_parent<RecordDecl>(d);
        if (decl_ref_idx(parent) && !decl_ptr(parent)->_decl_struct._link) {
            decl_ptr(parent)->_decl_struct._link = decl_ref_idx(r);
        }

        last = r;

        return true;
    }

    bool VisitEnumDecl(EnumDecl *d)
    {
        /* EnumDecl -> TagDecl -> TypeDecl -> NamedDecl -> Decl */

        if (d->isInvalidDecl()) return true;
        if (debug) print_decl(d);

        TagTypeKind k = d->clang::TagDecl::getTagKind();
        debugf("\tname:\"%s\" kind:%s neg_bits:%u pos_bits:%u\n",
            d->clang::NamedDecl::getNameAsString().c_str(),
            tagKindString(k),
            d->getNumNegativeBits(),
            d->getNumPositiveBits());

        /* create enum */
        decl_ref r = decl_new(db, _decl_enum, _top);
        decl_name_new(r, d->clang::NamedDecl::getNameAsString().c_str());
        idmap[d->clang::Decl::getID()] = decl_ref_idx(r);

        /* create prev next link */
        if (decl_ref_idx(last) && !decl_ptr(last)->_next) {
            decl_ptr(last)->_next = decl_ref_idx(r);
        }

        /* create parent link */
        decl_ref parent = get_parent<RecordDecl>(d);
        if (decl_ref_idx(parent) && !decl_ptr(parent)->_decl_struct._link) {
            decl_ptr(parent)->_decl_struct._link = decl_ref_idx(r);
        }

        stack.push_back(r);
        last = decl_ref { db, 0 };

        return true;
    }

    bool VisitEnumConstantDecl(EnumConstantDecl *d)
    {
        /* EnumConstantDecl -> ValueDecl -> NamedDecl -> Decl */

        if (d->isInvalidDecl()) return true;
        if (debug) print_decl(d);

        const QualType q = d->getType();
        uint64_t value = d->getInitVal().getExtValue();
        debugf("\tname:\"%s\" type:%s value:%" PRIu64 "\n",
            d->clang::NamedDecl::getNameAsString().c_str(),
            q.getAsString().c_str(), value);

        /* create constant */
        decl_ref r = decl_new(db, _decl_constant, _top);
        decl_name_new(r, d->clang::NamedDecl::getNameAsString().c_str());
        idmap[d->clang::Decl::getID()] = decl_ref_idx(r);
        decl_ptr(r)->_decl_constant._value = value;
        decl_ptr(r)->_decl_constant._decl = decl_ref_idx(get_intrinsic_type(q));

        /* create prev next link */
        if (decl_ref_idx(last) && !decl_ptr(last)->_next) {
            decl_ptr(last)->_next = decl_ref_idx(r);
        }

        /* create parent link */
        decl_ref parent = get_parent<EnumDecl>(d);
        if (decl_ref_idx(parent) && !decl_ptr(parent)->_decl_enum._link) {
            decl_ptr(parent)->_decl_enum._link = decl_ref_idx(r);
        }

        last = r;

        /* detect pop */
        Decl *nd = d->getNextDeclInContext();
        if (!nd && stack.size() > 0) {
            last = stack.back();
            stack.pop_back();
        }

        return true;
    }

    bool VisitRecordDecl(RecordDecl *d)
    {
        /* RecordDecl -> TagDecl -> TypeDecl -> NamedDecl -> Decl */

        if (d->isInvalidDecl()) return true;
        if (debug) print_decl(d);

        TagTypeKind k = d->clang::TagDecl::getTagKind();
        debugf("\tname:\"%s\" kind:%s\n",
            d->clang::NamedDecl::getNameAsString().c_str(), tagKindString(k));

        switch (k) {
        case TagTypeKind::TTK_Enum:
        case TagTypeKind::TTK_Class:
        case TagTypeKind::TTK_Interface:
            break;
        case TagTypeKind::TTK_Struct:
        case TagTypeKind::TTK_Union:
            {
                _Tag tag;
                switch (k) {
                case TagTypeKind::TTK_Struct: tag = _decl_struct; break;
                case TagTypeKind::TTK_Union:  tag = _decl_union;  break;
                default:                      tag = _decl_void;   break;
                }

                /* create struct */
                decl_ref r = decl_new(db, tag, _top);
                decl_name_new(r, d->clang::NamedDecl::getNameAsString().c_str());
                idmap[d->clang::Decl::getID()] = decl_ref_idx(r);

                /* create prev next link */
                if (decl_ref_idx(last) && !decl_ptr(last)->_next) {
                    decl_ptr(last)->_next = decl_ref_idx(r);
                }

                /* create parent link */
                decl_ref parent = get_parent<RecordDecl>(d);
                if (decl_ref_idx(parent) && !decl_ptr(parent)->_decl_struct._link) {
                    decl_ptr(parent)->_decl_struct._link = decl_ref_idx(r);
                }

                stack.push_back(r);
                last = decl_ref { db, 0 };

                break;
            }
        }

        return true;
    }

    bool VisitFieldDecl(FieldDecl *d)
    {
        /* FieldDecl -> DeclaratorDecl -> ValueDecl -> NamedDecl -> Decl */

        if (d->isInvalidDecl()) return true;
        if (debug) print_decl(d);

        const QualType q = d->getType();
        TypeInfo t = context.getTypeInfo(q);

        debugf("\tname:\"%s\" type:%s width:%" PRIu64 " align:%d index:%d\n",
            d->clang::NamedDecl::getNameAsString().c_str(),
            q.getAsString().c_str(), t.Width, t.Align, d->getFieldIndex());

        _Size width = d->isBitField() ? d->getBitWidthValue(context) : 0;

        /* create field */
        decl_ref r = decl_new(db, _decl_field, _top);
        decl_name_new(r, d->clang::NamedDecl::getNameAsString().c_str());
        idmap[d->clang::Decl::getID()] = decl_ref_idx(r);
        decl_ptr(r)->_attrs |= -d->isBitField() & _bitfield;
        decl_ptr(r)->_decl_field._decl = decl_ref_idx(get_intrinsic_type(q));
        decl_ptr(r)->_decl_field._width = width;

        /* create prev next link */
        if (decl_ref_idx(last) && !decl_ptr(last)->_next) {
            decl_ptr(last)->_next = decl_ref_idx(r);
        }

        /* create parent link */
        decl_ref parent = { db, idmap[d->getParent()->getID()] };
        if (!decl_ptr(parent)->_decl_struct._link) {
            decl_ptr(parent)->_decl_struct._link = decl_ref_idx(r);
        }

        last = r;

        /* detect pop */
        Decl *nd = d->getNextDeclInContext();
        if (!nd && stack.size() > 0) {
            last = stack.back();
            stack.pop_back();
        }

        return true;
    }

    bool VisitFunctionDecl(FunctionDecl *d)
    {
        /* FunctionDecl -> DeclaratorDecl -> ValueDecl -> NamedDecl -> Decl */

        if (d->isInvalidDecl()) return true;
        if (debug) print_decl(d);

        debugf("\tname:\"%s\" has_body:%d\n",
            d->getNameInfo().getName().getAsString().c_str(), d->hasBody());

        /* create constant */
        decl_ref r = decl_new(db, _decl_function, _top);
        decl_name_new(r, d->clang::NamedDecl::getNameAsString().c_str());
        idmap[d->clang::Decl::getID()] = decl_ref_idx(r);

        /* create prev next link */
        if (decl_ref_idx(last) && !decl_ptr(last)->_next) {
            decl_ptr(last)->_next = decl_ref_idx(r);
        }

        /* create parent link */
        decl_ref parent = get_parent<RecordDecl>(d);
        if (decl_ref_idx(parent) && !decl_ptr(parent)->_decl_struct._link) {
            decl_ptr(parent)->_decl_struct._link = decl_ref_idx(r);
        }

        last = r;

        QualType qr = d->getReturnType();

        /* create return param */
        decl_ref pr = decl_new(db, _decl_param, _top);
        decl_ptr(last)->_decl_function._link = decl_ref_idx(pr);
        decl_ptr(pr)->_decl_param._decl = decl_ref_idx(get_intrinsic_type(qr));

        /* create argument params */
        const ArrayRef<ParmVarDecl*> parms = d->parameters();
        for (size_t i = 0; i < parms.size(); i++) {
            const ParmVarDecl* parm = parms[i];
            const QualType q = parm->getOriginalType();

            decl_ref ar = decl_new(db, _decl_param, _top);
            decl_name_new(ar, parm->clang::NamedDecl::getNameAsString().c_str());
            decl_ptr(pr)->_next = decl_ref_idx(ar);
            decl_ptr(ar)->_decl_param._decl = decl_ref_idx(get_intrinsic_type(q));
            pr = ar;
        }

        return true;
    }

    bool VisitVarDecl(VarDecl *d)
    {
        /* VarDecl -> DeclaratorDecl -> ValueDecl -> NamedDecl -> Decl */

        if (d->isInvalidDecl()) return true;
        if (debug) print_decl(d);

        debugf("\tname:\"%s\"\n",
            d->clang::NamedDecl::getNameAsString().c_str());

        return true;
    }
};

void crefl::CReflectAction::EndSourceFileAction()
{
    auto &ci      = getCompilerInstance();
    auto &context = ci.getASTContext();
    auto &input = getCurrentInput();

    decl_db *db = decl_db_new();
    decl_db_defaults(db);
    CReflectVisitor v(context, db);

    llvm::StringRef fileName = input.getFile();
    if (v.debug) log_debug("File %s\n", fileName.str().c_str());

    v.TraverseDecl(context.getTranslationUnitDecl());
    decl_db_dump(db);
    decl_db_destroy(db);

    clang::ASTFrontendAction::EndSourceFileAction();
}
