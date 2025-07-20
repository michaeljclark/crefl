/*
 * cinf runtime library and compiler plug-in to support reflection in C.
 *
 * Copyright (c) 2020-2022 Michael Clark <michaeljclark@mac.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <cstdio>
#include <cstdarg>
#include <cinttypes>

#include <string>
#include <vector>

#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ParentMapContext.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Frontend/CompilerInstance.h>

#include <cinf/util.h>
#include <cinf/model.h>
#include <cinf/dump.h>
#include <cinf/db.h>

using namespace clang;

class ReflectAction : public PluginASTAction
{
public:
    std::string outputFile;
    bool debug, dump;

    ReflectAction() : outputFile(), debug(false), dump(false) {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer
        (CompilerInstance &ci, llvm::StringRef) override {
        ci.getDiagnostics().setClient(new IgnoringDiagConsumer());
        return std::make_unique<ASTConsumer>();
    }

    bool ParseArgs(const CompilerInstance &ci,
                 const std::vector<std::string>& argv) override;

protected:
    void EndSourceFileAction() override;
    PluginASTAction::ActionType getActionType() override {
      return ReplaceAction;
    }
};

static FrontendPluginRegistry::Add<ReflectAction>
    X("cinfcc", "emit reflection metadata.");

static void log_debug(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    std::string buf = string_vprintf(fmt, args);
    va_end(args);
    fwrite(buf.c_str(), 1, buf.size(), stderr);
}

#define debugf(...) if(debug) log_debug(__VA_ARGS__)

static const char* tagKindString(TagTypeKind k) {
    switch (k) {
    case TagTypeKind::Struct: return "struct";
    case TagTypeKind::Interface: return "interface";
    case TagTypeKind::Union: return "union";
    case TagTypeKind::Class: return "class";
    case TagTypeKind::Enum: return "enum";
    default: return "unknown";
    }
}

struct ReflectVisitor : public RecursiveASTVisitor<ReflectVisitor>
{
    ASTContext &context;
    decl_db *db;
    decl_ref last;
    llvm::SmallVector<decl_ref,16> stack;
    llvm::SmallMapVector<int64_t,decl_id, 16> idmap;
    std::string inputFile;
    bool debug;

    ReflectVisitor(ASTContext &context, decl_db *db, std::string inputFile, bool debug = false)
        : context(context), db(db), last(), inputFile(inputFile), debug(debug) {}

    std::string cinf_path(const Decl *d)
    {
        std::string s;
        std::vector<std::string> dl;
        dl.push_back(d->getDeclKindName());
        const DeclContext *dc = d->getLexicalDeclContext();
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

    void print_decl(const Decl *d)
    {
        std::string dps = cinf_path(d);
        SourceLocation sl = d->getLocation();
        std::string sls = sl.printToString(context.getSourceManager());
        const Decl *nd = d->getNextDeclInContext();
        auto module = d->getLocalOwningModule();
        std::string mod = module ? module->getFullModuleName(true) : "";
        if (nd) {
            log_debug("(%" PRId64 " -> %" PRId64 ") %s : %s : module=%s\n",
                d->getID(), nd->getID(), dps.c_str(), sls.c_str(), mod.c_str());
        } else {
            log_debug("(%" PRId64 ") %s : %s : module=%s\n",
                d->getID(), dps.c_str(), sls.c_str(), mod.c_str());
        }
    }

    decl_ref create_attribute(decl_ref last, const char *name)
    {
        /* create attribute */
        decl_ref r = cinf_decl_new(db, decl_attribute);
        cinf_decl_ptr(r)->name = cinf_name_new(db, name);

        /* create prev next link */
        if (cinf_decl_idx(last) && !cinf_decl_ptr(last)->next) {
            cinf_decl_ptr(last)->next = cinf_decl_idx(r);
        }

        return r;
    }

    decl_ref create_attribute_string(decl_ref last, const char *name, const char *val)
    {
        /* create attribute */
        decl_ref r = cinf_decl_new(db, decl_attribute);
        cinf_decl_ptr(r)->name = cinf_name_new(db, name);

        /* create value */
        decl_ref v = cinf_decl_new(db, decl_value);
        cinf_decl_ptr(v)->name = cinf_name_new(db, val);
        cinf_decl_ptr(r)->link = cinf_decl_idx(v);

        /* create prev next link */
        if (cinf_decl_idx(last) && !cinf_decl_ptr(last)->next) {
            cinf_decl_ptr(last)->next = cinf_decl_idx(r);
        }

        return r;
    }

    decl_ref create_attribute_value(decl_ref last, const char *name, decl_sz val)
    {
        /* create attribute */
        decl_ref r = cinf_decl_new(db, decl_attribute);
        cinf_decl_ptr(r)->name = cinf_name_new(db, name);

        /* create value */
        decl_ref v = cinf_decl_new(db, decl_value);
        cinf_decl_ptr(v)->value = val;
        cinf_decl_ptr(r)->link = cinf_decl_idx(v);

        /* create prev next link */
        if (cinf_decl_idx(last) && !cinf_decl_ptr(last)->next) {
            cinf_decl_ptr(last)->next = cinf_decl_idx(r);
        }

        return r;
    }

    decl_id create_attributes(Decl *d, decl_ref r)
    {
        decl_ref last = { 0 };
        decl_id result = 0;

        AttrVec& av = d->getAttrs();
        for (auto &at : av) {
            if (at->getAttrName()) {
                debugf("\tattribute:%s\n", at->getAttrName()->getNameStart());
            }
            if (isa<AlwaysInlineAttr>(*at)) {
                last = create_attribute(last, "always_inline");
            }
            else if (isa<DeprecatedAttr>(*at)) {
                last = create_attribute(last, "deprecated");
            }
            else if (isa<PureAttr>(*at)) {
                last = create_attribute(last, "pure");
            }
            else if (isa<PackedAttr>(*at)) {
                last = create_attribute(last, "packed");
            }
            else if (isa<UsedAttr>(*at)) {
                last = create_attribute(last, "used");
            }
            else if (isa<UnusedAttr>(*at)) {
                last = create_attribute(last, "unused");
            }
            else if (isa<WarnUnusedResultAttr>(*at)) {
                last = create_attribute(last, "warn_unused_result");
            }
            else if (isa<AliasAttr>(*at)) {
                std::string aliasee = cast<AliasAttr>(*at).getAliasee().str();
                last = create_attribute_string(last, "alias", aliasee.c_str());
            }
            else if (isa<AlignedAttr>(*at)) {
                unsigned align = cast<AlignedAttr>(*at).getAlignment(context);
                last = create_attribute_value(last, "aligned", align);
            }
            else if (isa<AnnotateAttr>(*at)) {
                std::string annotation = cast<AnnotateAttr>(*at).getAnnotation().str();
                last = create_attribute_string(last, "annotate", annotation.c_str());
            }
            if (cinf_decl_idx(last)) result = cinf_decl_idx(last);
        }
        return result;
    }

    decl_ref get_intrinsic_type(const QualType q)
    {
        decl_ref tr = cinf_intrinsic(db, decl_void, 0);

        bool _is_scalar = q->isScalarType();
        bool _is_pointer = q->isPointerType();
        bool _is_complex = q->isComplexType();
        bool _is_vector = q->isVectorType();
        bool _is_array = q->isArrayType();
        bool _is_union = q->isUnionType();
        bool _is_struct = q->isStructureOrClassType();
        bool _is_enum = q->isEnumeralType();
        bool _is_const = (q.getLocalFastQualifiers() & Qualifiers::Const);
        bool _is_restrict = (q.getLocalFastQualifiers() & Qualifiers::Restrict);
        bool _is_volatile = (q.getLocalFastQualifiers() & Qualifiers::Volatile);

        if (!_is_array && q->isIncompleteType()) return tr;

        TypeInfo t = context.getTypeInfo(q);

        if (_is_pointer) {
            const QualType pq = q->getPointeeType();
            decl_ref ti = get_intrinsic_type(pq);

            std::string name = string_printf("*%s", cinf_decl_name(ti));

            tr = cinf_decl_new(db, decl_pointer);
            cinf_decl_ptr(tr)->link = cinf_decl_idx(ti);
            cinf_decl_ptr(tr)->name = cinf_name_new(db, name.c_str());
            cinf_decl_ptr(tr)->width = t.Width;
        }
        else if (_is_enum) {
            const EnumType *et = q->getAs<EnumType>();
            const EnumDecl *ed = et->getDecl();
            tr = decl_ref { db, idmap[ed->getID()] };
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
        else if (_is_complex) {
            tr = cinf_intrinsic(db, decl_cfloat, t.Width);
        }
        else if (_is_scalar) {
            auto stk = q->getScalarTypeKind();
            switch (stk) {
            case Type::ScalarTypeKind::STK_CPointer: {
                 tr = cinf_intrinsic(db, decl_void, t.Width);
                 break;
            }
            case Type::ScalarTypeKind::STK_BlockPointer:
                break;
            case Type::ScalarTypeKind::STK_ObjCObjectPointer:
                break;
            case Type::ScalarTypeKind::STK_MemberPointer:
                break;
            case Type::ScalarTypeKind::STK_Bool: {
                tr = cinf_intrinsic(db, decl_int, 1);
                break;
            }
            case Type::ScalarTypeKind::STK_Integral: {
                if (q->isUnsignedIntegerType()) {
                    tr = cinf_intrinsic(db, decl_uint, t.Width);
                } else {
                    tr = cinf_intrinsic(db, decl_int, t.Width);
                }
                break;
            }
            case Type::ScalarTypeKind::STK_Floating: {
                tr = cinf_intrinsic(db, decl_float, t.Width);
                break;
            }
            case Type::ScalarTypeKind::STK_IntegralComplex:
                break;
            case Type::ScalarTypeKind::STK_FloatingComplex:
                break;
            case Type::ScalarTypeKind::STK_FixedPoint:
                break;
            }
        }
        else if (_is_array) {
            /* also {Incomplete,DependentSized,Variable}ArrayType */
            const ConstantArrayType * cat = context.getAsConstantArrayType(q);
            const VariableArrayType * vat = context.getAsVariableArrayType(q);
            const ArrayType *at = context.getAsArrayType(q);
            const QualType q = at->getElementType();
            bool is_static = at->getSizeModifier() == ArraySizeModifier::Static;
            std::string name;

            decl_ref ti = get_intrinsic_type(q);

            tr = cinf_decl_new(db, decl_array);
            cinf_decl_ptr(tr)->link = cinf_decl_idx(ti);
            const char *decl_name = cinf_decl_name(ti);
            if (vat) {
                name = string_printf("[*]%s", decl_name);
                cinf_decl_ptr(tr)->props |= decl_vla;
            } else if (cat) {
                u64 _count = cat->getSize().getLimitedValue();
                cinf_decl_ptr(tr)->count = _count;
                name = string_printf("[%llu]%s", _count, decl_name);
            } else {
                name = string_printf("[]%s", decl_name);
            }
            cinf_decl_ptr(tr)->name = cinf_name_new(db, name.c_str());
            cinf_decl_ptr(tr)->props |= -(int)is_static & decl_static;
        }
        else if (_is_vector) {
            const VectorType *vt = q->getAs<VectorType>();
            // TODO
        }
        else {
            tr = cinf_intrinsic(db, decl_void, t.Width);
        }

        debugf("\tscalar:%d complex:%d vector:%d array:%d struct:%d"
               " union:%d const:%d volatile:%d restrict:%d\n",
            _is_scalar, _is_complex, _is_vector, _is_array, _is_struct,
            _is_union, _is_const, _is_volatile, _is_restrict);

        int cvr_props = 0;
        cvr_props |= decl_const & -(int)_is_const;
        cvr_props |= decl_volatile & -(int)_is_volatile;
        cvr_props |= decl_restrict & -(int)_is_restrict;

        if (cvr_props) {
            decl_ref ti = tr;
            tr = cinf_decl_new(db, decl_qualifier);
            std::string name = string_printf("%s%s%s%s",
                _is_const ? "const_" : "",
                _is_volatile ? "volatile_" : "",
                _is_restrict ? "restrict_" : "",
                cinf_decl_name(ti));
            cinf_decl_ptr(tr)->name = cinf_name_new(db, name.c_str());
            cinf_decl_ptr(tr)->link = cinf_decl_idx(ti);
            cinf_decl_ptr(tr)->props |= cvr_props;
        }

        return tr;
    }

    decl_ref create_named_node(NamedDecl *d, decl_tag tag)
    {
        /* create node */
        decl_ref r = cinf_decl_new(db, tag);
        cinf_decl_ptr(r)->name = cinf_name_new(db,
            d->NamedDecl::getNameAsString().c_str());

        /* record clang -> cinf id */
        idmap[d->Decl::getID()] = cinf_decl_idx(r);

        /* find parent id */
        const auto& parents = context.getParents(*d);
        decl_ref p = {
            db, parents.size() > 0 ? idmap[parents[0].get<Decl>()->getID()] : 0
        };

        /* pop until parent matches */
        while (stack.size() > 0 && stack.back().decl_idx != p.decl_idx) {
            last = stack.back();
            stack.pop_back();
        }

        /* create next last links */
        if (cinf_decl_idx(last) && !cinf_decl_ptr(last)->next) {
            cinf_decl_ptr(last)->next = cinf_decl_idx(r);
        }

        /* create parent link */
        if (cinf_decl_idx(p) && !cinf_decl_ptr(p)->link) {
            cinf_decl_ptr(p)->link = cinf_decl_idx(r);
        }

        /* set root element */
        if (db->root_element == 0) {
            db->root_element = cinf_decl_idx(r);
        }

        return r;
    }

    decl_set get_cvr_props(QualType q)
    {
        int props = 0;
        props |= decl_const & -(int)(q.getLocalFastQualifiers() & Qualifiers::Const);
        props |= decl_volatile & -(int)(q.getLocalFastQualifiers() & Qualifiers::Volatile);
        props |= decl_restrict & -(int)(q.getLocalFastQualifiers() & Qualifiers::Restrict);
        return props;
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

        /* create node */
        decl_ref r = cinf_decl_new(db, decl_source);
        cinf_decl_ptr(r)->name = cinf_name_new(db,
            cinf_basename(inputFile).c_str());

        /* record clang -> cinf id */
        idmap[d->Decl::getID()] = cinf_decl_idx(r);

        /* set root element */
        if (db->root_element == 0) {
            db->root_element = cinf_decl_idx(r);
        }

        return true;
    }

    bool VisitTypedefDecl(TypedefDecl *d)
    {
        /* TypedefDecl -> TypeNameDecl -> TypeDecl -> NamedDecl -> Decl */

        if (d->isInvalidDecl()) return true;
        if (debug) print_decl(d);

        const QualType q = d->getUnderlyingType();
        debugf("\tname:\"%s\" type:%s\n",
            d->NamedDecl::getNameAsString().c_str(),
            q.getAsString().c_str());

        /* create typedef */
        decl_ref r = create_named_node(d, decl_typedef);
        cinf_decl_ptr(r)->link = cinf_decl_idx(get_intrinsic_type(q));
        cinf_decl_ptr(r)->attr = create_attributes(d, r);

        last = r;

        return true;
    }

    bool VisitEnumDecl(EnumDecl *d)
    {
        /* EnumDecl -> TagDecl -> TypeDecl -> NamedDecl -> Decl */

        if (d->isInvalidDecl()) return true;
        if (debug) print_decl(d);

        TagTypeKind k = d->TagDecl::getTagKind();
        debugf("\tname:\"%s\" kind:%s neg_bits:%u pos_bits:%u\n",
            d->NamedDecl::getNameAsString().c_str(),
            tagKindString(k),
            d->getNumNegativeBits(),
            d->getNumPositiveBits());

        const QualType q = d->getIntegerType();
        TypeInfo t = context.getTypeInfo(q);

        /* create enum */
        decl_ref r = create_named_node(d, decl_enum);
        cinf_decl_ptr(r)->attr = create_attributes(d, r);
        cinf_decl_ptr(r)->width = t.Width;

        stack.push_back(r);
        last = decl_ref { db, 0 };

        return true;
    }

    bool VisitEnumConstantDecl(EnumConstantDecl *d)
    {
        /* EnumConstantDecl -> ValueDecl -> NamedDecl -> Decl */

        if (d->isInvalidDecl()) return true;
        if (debug) print_decl(d);

        const EnumDecl *ed = context.getParents(*d)[0].get<EnumDecl>();
        const QualType q = const_cast<EnumDecl*>(ed)->getIntegerType();

        uint64_t value = d->getInitVal().extOrTrunc(64).getZExtValue();
        debugf("\tname:\"%s\" type:%s value:%" PRIu64 "\n",
            d->NamedDecl::getNameAsString().c_str(),
            q.getAsString().c_str(), value);

        /* create constant */
        decl_ref r = create_named_node(d, decl_constant);
        cinf_decl_ptr(r)->link = cinf_decl_idx(get_intrinsic_type(q));
        cinf_decl_ptr(r)->value = value;
        cinf_decl_ptr(r)->attr = create_attributes(d, r);

        last = r;

        return true;
    }

    bool VisitRecordDecl(RecordDecl *d)
    {
        /* RecordDecl -> TagDecl -> TypeDecl -> NamedDecl -> Decl */

        if (d->isInvalidDecl()) return true;
        if (debug) print_decl(d);

        TagTypeKind k = d->TagDecl::getTagKind();
        debugf("\tname:\"%s\" kind:%s\n",
            d->NamedDecl::getNameAsString().c_str(), tagKindString(k));

        decl_tag tag;
        decl_ref r;

        switch (k) {
        case TagTypeKind::Enum:
        case TagTypeKind::Class:
        case TagTypeKind::Interface:
            break;

        case TagTypeKind::Struct:
        case TagTypeKind::Union:

            switch (k) {
            case TagTypeKind::Struct: tag = decl_struct; break;
            case TagTypeKind::Union:  tag = decl_union;  break;
            default:                  tag = decl_none;   break;
            }

            /* create struct */
            decl_ref r = create_named_node(d, tag);
            cinf_decl_ptr(r)->attr = create_attributes(d, r);

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
            d->NamedDecl::getNameAsString().c_str(),
            q.getAsString().c_str(), t.Width, t.Align, d->getFieldIndex());

        decl_sz width = d->isBitField() ? d->getBitWidthValue(context) : 0;

        /* create field */
        decl_ref r = create_named_node(d, decl_field);
        cinf_decl_ptr(r)->link = cinf_decl_idx(get_intrinsic_type(q));
        cinf_decl_ptr(r)->width = width;
        cinf_decl_ptr(r)->props |= (-(int)d->isBitField() & decl_bitfield);
        cinf_decl_ptr(r)->attr = create_attributes(d, r);

        last = r;

        return true;
    }

    bool VisitFunctionDecl(FunctionDecl *d)
    {
        /* FunctionDecl -> DeclaratorDecl -> ValueDecl -> NamedDecl -> Decl */

        if (d->isInvalidDecl()) return true;
        if (debug) print_decl(d);

        debugf("\tname:\"%s\" has_body:%d\n",
            d->getNameInfo().getName().getAsString().c_str(), d->hasBody());

        /* create function */
        decl_ref r = create_named_node(d, decl_function);
        cinf_decl_ptr(r)->attr = create_attributes(d, r);
        cinf_decl_ptr(r)->props |=
            (-(int)d->isGlobal() & decl_global) |
            (-(int)d->isExternC() & decl_extern_c) |
            (-(int)d->isStatic() & decl_static) |
            (-(int)d->isInlined() & decl_inline) |
            (-(int)d->isNoReturn() & decl_noreturn);

        last = r;

        QualType qr = d->getReturnType();

        /* create return param */
        decl_ref pr = cinf_decl_new(db, decl_parameter);
        cinf_decl_ptr(last)->link = cinf_decl_idx(pr);
        cinf_decl_ptr(pr)->link = cinf_decl_idx(get_intrinsic_type(qr));
        cinf_decl_ptr(pr)->props |= decl_out;

        /* create argument params */
        const ArrayRef<ParmVarDecl*> parms = d->parameters();
        for (size_t i = 0; i < parms.size(); i++) {
            const ParmVarDecl* parm = parms[i];
            const QualType q = parm->getOriginalType();

            decl_ref ar = cinf_decl_new(db, decl_parameter);
            cinf_decl_ptr(ar)->name = cinf_name_new(db,
                parm->NamedDecl::getNameAsString().c_str());
            cinf_decl_ptr(pr)->next = cinf_decl_idx(ar);
            cinf_decl_ptr(ar)->link = cinf_decl_idx(get_intrinsic_type(q));
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
            d->NamedDecl::getNameAsString().c_str());

        /* FunctionDecl handles params */
        if (d->isLocalVarDeclOrParm()) return true;

        /* create field */
        decl_ref r = create_named_node(d, decl_field);
        const QualType q = d->getTypeSourceInfo()->getType();
        bool is_static = d->getStorageDuration() == StorageDuration::SD_Static;
        cinf_decl_ptr(r)->link = cinf_decl_idx(get_intrinsic_type(q));
        cinf_decl_ptr(r)->props |=
            (-(int)d->isExternC() & decl_extern_c) |
            (-(int)is_static & decl_static);
        cinf_decl_ptr(r)->attr = create_attributes(d, r);

        last = r;

        return true;
    }
};

bool ReflectAction::ParseArgs(const CompilerInstance &ci,
             const std::vector<std::string>& argv)
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

void ReflectAction::EndSourceFileAction()
{
    auto &ci      = getCompilerInstance();
    auto &context = ci.getASTContext();
    auto &input = getCurrentInput();

    decl_db *db = cinf_db_new();
    cinf_db_defaults(db);
    ReflectVisitor v(context, db, input.getFile().str(), debug);
    if (debug) {
        log_debug("Input file  : %s\n", v.inputFile.c_str());
        if (outputFile.size() != 0) {
            log_debug("Output file : %s\n", outputFile.c_str());
        }
    }

    v.TraverseDecl(context.getTranslationUnitDecl());
    if (dump) {
        cinf_db_dump(db);
    }
    if (outputFile.size()) {
        cinf_db_write_file(db, outputFile.c_str());
    }
    cinf_db_destroy(db);

    ASTFrontendAction::EndSourceFileAction();
}
