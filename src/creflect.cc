#include <cstdio>
#include <cstdarg>
#include <cinttypes>

#include <vector>

#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ParentMapContext.h"
#include <clang/Frontend/FrontendPluginRegistry.h>

#include "cmodel.h"
#include "cfileio.h"
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
    llvm::SmallMapVector<int64_t,decl_id, 16> idmap;
    bool debug;

    CReflectVisitor(ASTContext &context, decl_db *db, bool debug = false)
        : context(context), db(db), last(), debug(debug) {}

    std::string crefl_path(Decl *d)
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
        std::string dps = crefl_path(d);
        SourceLocation sl = d->getLocation();
        std::string sls = sl.printToString(context.getSourceManager());
        Decl *nd = d->getNextDeclInContext();
        if (nd) {
            log_debug("(%" PRId64 " -> %" PRId64 ") %s : %s\n",
                d->getID(), nd->getID(), dps.c_str(), sls.c_str());
        } else {
            log_debug("(%" PRId64 ") %s : %s\n",
                d->getID(), dps.c_str(), sls.c_str());
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
                 tr = crefl_find_intrinsic(db, _top | _void, t.Width);
                 break;
            }
            case clang::Type::ScalarTypeKind::STK_BlockPointer:
                break;
            case clang::Type::ScalarTypeKind::STK_ObjCObjectPointer:
                break;
            case clang::Type::ScalarTypeKind::STK_MemberPointer:
                break;
            case clang::Type::ScalarTypeKind::STK_Bool: {
                tr = crefl_find_intrinsic(db, _top | _sint, 1);
                break;
            }
            case clang::Type::ScalarTypeKind::STK_Integral: {
                bool _is_unsigned = q->isUnsignedIntegerType();
                tr = crefl_find_intrinsic(db, _top |
                    (_is_unsigned ? _uint : _sint), t.Width);
                break;
            }
            case clang::Type::ScalarTypeKind::STK_Floating: {
                tr = crefl_find_intrinsic(db, _top | _float, t.Width);
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

            tr = crefl_new(db, _decl_array, _top);
            crefl_ptr(tr)->_decl_array._decl = crefl_ref_idx(ti);
            if (cat) {
                u64 _size = cat->getSize().getLimitedValue();
                crefl_ptr(tr)->_decl_array._size = _size;
                snprintf(namebuf, sizeof(namebuf), "%s[%llu]",
                    crefl_name(ti), _size);
            } else {
                snprintf(namebuf, sizeof(namebuf), "%s[]",
                    crefl_name(ti));
            }
            crefl_name_new(tr, namebuf);
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
        decl_ref r = crefl_new(db, _decl_typedef, _top);
        crefl_name_new(r, d->clang::NamedDecl::getNameAsString().c_str());
        idmap[d->clang::Decl::getID()] = crefl_ref_idx(r);
        crefl_ptr(r)->_decl_typedef._decl = crefl_ref_idx(get_intrinsic_type(q));

        /* create prev next link */
        if (crefl_ref_idx(last) && !crefl_ptr(last)->_next) {
            crefl_ptr(last)->_next = crefl_ref_idx(r);
        }

        /* create parent link */
        decl_ref p = get_parent<RecordDecl>(d);
        if (crefl_ref_idx(p) && !crefl_ptr(p)->_decl_struct._link) {
            crefl_ptr(p)->_decl_struct._link = crefl_ref_idx(r);
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
        decl_ref r = crefl_new(db, _decl_enum, _top);
        crefl_name_new(r, d->clang::NamedDecl::getNameAsString().c_str());
        idmap[d->clang::Decl::getID()] = crefl_ref_idx(r);

        /* create prev next link */
        if (crefl_ref_idx(last) && !crefl_ptr(last)->_next) {
            crefl_ptr(last)->_next = crefl_ref_idx(r);
        }

        /* create parent link */
        decl_ref p = get_parent<RecordDecl>(d);
        if (crefl_ref_idx(p) && !crefl_ptr(p)->_decl_struct._link) {
            crefl_ptr(p)->_decl_struct._link = crefl_ref_idx(r);
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
        decl_ref r = crefl_new(db, _decl_constant, _top);
        crefl_name_new(r, d->clang::NamedDecl::getNameAsString().c_str());
        idmap[d->clang::Decl::getID()] = crefl_ref_idx(r);
        crefl_ptr(r)->_decl_constant._value = value;
        crefl_ptr(r)->_decl_constant._decl = crefl_ref_idx(get_intrinsic_type(q));

        /* create prev next link */
        if (crefl_ref_idx(last) && !crefl_ptr(last)->_next) {
            crefl_ptr(last)->_next = crefl_ref_idx(r);
        }

        /* create parent link */
        decl_ref p = get_parent<EnumDecl>(d);
        if (crefl_ref_idx(p) && !crefl_ptr(p)->_decl_enum._link) {
            crefl_ptr(p)->_decl_enum._link = crefl_ref_idx(r);
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

        decl_tag tag;
        decl_ref r, p;

        switch (k) {
        case TagTypeKind::TTK_Enum:
        case TagTypeKind::TTK_Class:
        case TagTypeKind::TTK_Interface:
            break;

        case TagTypeKind::TTK_Struct:
        case TagTypeKind::TTK_Union:

            switch (k) {
            case TagTypeKind::TTK_Struct: tag = _decl_struct; break;
            case TagTypeKind::TTK_Union:  tag = _decl_union;  break;
            default:                      tag = _decl_void;   break;
            }

            /* create struct */
            r = crefl_new(db, tag, _top);
            crefl_name_new(r, d->clang::NamedDecl::getNameAsString().c_str());
            idmap[d->clang::Decl::getID()] = crefl_ref_idx(r);

            /* create prev next link */
            if (crefl_ref_idx(last) && !crefl_ptr(last)->_next) {
                crefl_ptr(last)->_next = crefl_ref_idx(r);
            }

            /* create parent link */
            p = get_parent<RecordDecl>(d);
            if (crefl_ref_idx(p) && !crefl_ptr(p)->_decl_struct._link) {
                crefl_ptr(p)->_decl_struct._link = crefl_ref_idx(r);
            }

            stack.push_back(r);
            last = decl_ref { db, 0 };

            break;
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

        decl_sz width = d->isBitField() ? d->getBitWidthValue(context) : 0;

        /* create field */
        decl_ref r = crefl_new(db, _decl_field, _top);
        crefl_name_new(r, d->clang::NamedDecl::getNameAsString().c_str());
        idmap[d->clang::Decl::getID()] = crefl_ref_idx(r);
        crefl_ptr(r)->_attrs |= -d->isBitField() & _bitfield;
        crefl_ptr(r)->_decl_field._decl = crefl_ref_idx(get_intrinsic_type(q));
        crefl_ptr(r)->_decl_field._width = width;

        /* create prev next link */
        if (crefl_ref_idx(last) && !crefl_ptr(last)->_next) {
            crefl_ptr(last)->_next = crefl_ref_idx(r);
        }

        /* create parent link */
        decl_ref p = { db, idmap[d->getParent()->getID()] };
        if (!crefl_ptr(p)->_decl_struct._link) {
            crefl_ptr(p)->_decl_struct._link = crefl_ref_idx(r);
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
        decl_ref r = crefl_new(db, _decl_function, _top);
        crefl_name_new(r, d->clang::NamedDecl::getNameAsString().c_str());
        idmap[d->clang::Decl::getID()] = crefl_ref_idx(r);

        /* create prev next link */
        if (crefl_ref_idx(last) && !crefl_ptr(last)->_next) {
            crefl_ptr(last)->_next = crefl_ref_idx(r);
        }

        /* create parent link */
        decl_ref p = get_parent<RecordDecl>(d);
        if (crefl_ref_idx(p) && !crefl_ptr(p)->_decl_struct._link) {
            crefl_ptr(p)->_decl_struct._link = crefl_ref_idx(r);
        }

        last = r;

        QualType qr = d->getReturnType();

        /* create return param */
        decl_ref pr = crefl_new(db, _decl_param, _top);
        crefl_ptr(last)->_decl_function._link = crefl_ref_idx(pr);
        crefl_ptr(pr)->_decl_param._decl = crefl_ref_idx(get_intrinsic_type(qr));

        /* create argument params */
        const ArrayRef<ParmVarDecl*> parms = d->parameters();
        for (size_t i = 0; i < parms.size(); i++) {
            const ParmVarDecl* parm = parms[i];
            const QualType q = parm->getOriginalType();

            decl_ref ar = crefl_new(db, _decl_param, _top);
            crefl_name_new(ar, parm->clang::NamedDecl::getNameAsString().c_str());
            crefl_ptr(pr)->_next = crefl_ref_idx(ar);
            crefl_ptr(ar)->_decl_param._decl = crefl_ref_idx(get_intrinsic_type(q));
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

        // TODO

        return true;
    }
};

crefl::CReflectAction::CReflectAction()
    : outputFile(), debug(false), dump(false) {}

std::unique_ptr<clang::ASTConsumer> crefl::CReflectAction::CreateASTConsumer
    (clang::CompilerInstance &ci, llvm::StringRef)
{
    ci.getDiagnostics().setClient(new clang::IgnoringDiagConsumer());
    return std::make_unique<clang::ASTConsumer>();
}

bool crefl::CReflectAction::ParseArgs
    (const clang::CompilerInstance &ci, const std::vector<std::string>& argv)
{
    int i = 0, argc = argv.size();
    while (i < argc) {
        if (argv[i] == "-o") {
            if (++i == argc) {
                fprintf(stderr, "error: -o requires parameter\n");
                exit(0);
            }
            outputFile = argv[i++];
        } else if (argv[i] == "-debug") {
            ++i; debug = true;
        } else if (argv[i] == "-dump") {
            ++i; dump = true;
        } else {
            fprintf(stderr, "error: unknown option: %s\n", argv[i].c_str());
            exit(0);
        }
    }
    if (!dump && !debug && !outputFile.size()) {
        fprintf(stderr, "error: missing args: -dump, -debug, -o <outputFile>\n");
        exit(0);
    }
    return true;
}

void crefl::CReflectAction::EndSourceFileAction()
{
    auto &ci      = getCompilerInstance();
    auto &context = ci.getASTContext();
    auto &input = getCurrentInput();

    decl_db *db = crefl_db_new();
    crefl_db_defaults(db);
    CReflectVisitor v(context, db, debug);

    llvm::StringRef fileName = input.getFile();
    if (debug) {
        log_debug("Input file  : %s\n", fileName.str().c_str());
        if (outputFile.size() != 0) {
            log_debug("Output file : %s\n", outputFile.c_str());
        }
    }

    v.TraverseDecl(context.getTranslationUnitDecl());
    if (dump) {
        crefl_db_dump(db);
    }
    if (outputFile.size()) {
        crefl_write_db(db, outputFile.c_str());
    }
    crefl_db_destroy(db);

    clang::ASTFrontendAction::EndSourceFileAction();
}
