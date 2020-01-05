#include "fmt/core.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"

namespace fmt {
    template<>
    struct formatter<cc0::ErrorCode> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

        template<typename FormatContext>
        auto format(const cc0::ErrorCode &p, FormatContext &ctx) {
            std::string name;
            switch (p) {
                case cc0::ErrNoError:
                    name = "No error.";
                    break;
                case cc0::ErrStreamError:
                    name = "Stream error.";
                    break;
                case cc0::ErrEOF:
                    name = "EOF";
                    break;
                case cc0::ErrInvalidInput:
                    name = "The input is invalid.";
                    break;
                case cc0::ErrInvalidIdentifier:
                    name = "Identifier is invalid";
                    break;
                case cc0::ErrIntegerOverflow:
                    name = "The integer is too big(int64_t).";
                    break;
                case cc0::ErrNoBegin:
                    name = "The program should start with 'begin'.";
                    break;
                case cc0::ErrNoEnd:
                    name = "The program should end with 'end'.";
                    break;
                case cc0::ErrNeedIdentifier:
                    name = "Need an identifier here.";
                    break;
                case cc0::ErrConstantNeedValue:
                    name = "The constant need a value to initialize.";
                    break;
                case cc0::ErrNoSemicolon:
                    name = "Zai? Wei shen me bu xie fen hao.";
                    break;
                case cc0::ErrInvalidVariableDeclaration:
                    name = "The declaration is invalid.";
                    break;
                case cc0::ErrIncompleteExpression:
                    name = "The expression is incomplete.";
                    break;
                case cc0::ErrNotDeclared:
                    name = "The variable or constant must be declared before being used.";
                    break;
                case cc0::ErrAssignToConstant:
                    name = "Trying to assign value to a constant.";
                    break;
                case cc0::ErrDuplicateDeclaration:
                    name = "The variable or constant has been declared.";
                    break;
                case cc0::ErrNotInitialized:
                    name = "The variable has not been initialized.";
                    break;
                case cc0::ErrInvalidAssignment:
                    name = "The assignment statement is invalid.";
                    break;
                case cc0::ErrInvalidPrint:
                    name = "The output statement is invalid.";
                    break;
                case cc0::ErrNoRightBracket:
                    name = "Expression has ( but you forget ) ";
                    break;
                case cc0::ErrNoCommentEnd:
                    name = "Code has /* but you forget */ ";
                    break;
                case cc0::ErrNeedVarType:
                    name = "var need  a type";
                    break;
                case cc0::ErrFunctionDeclared:
                    name = "error Function Declared ";
                    break;
                case cc0::ErrFunctionParams:
                    name = "error Function params ";
                    break;
                case cc0::ErrConditionStatement:
                    name = "error Condition Statement ";
                    break;
                case cc0::ErrLoopStatement:
                    name = "error Loop Statement ";
                    break;
                case cc0::ErrJumpStatement:
                    name = "error Jump Statement ";
                    break;
                case cc0::ErrPrintStatement:
                    name = "error Print Statement";
                    break;
                case cc0::ErrFunctionCall:
                    name = "error Function Call ";
                    break;
                case cc0::ErrNeedMain:
                    name = "you should declare main function .";
                    break;
                case cc0::ErrViod:
                    name = "you can't use void in expression";
                    break;

                default:
                    name = "error but lazy write why";
            }
            return format_to(ctx.out(), name);
        }
    };

    template<>
    struct formatter<cc0::CompilationError> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

        template<typename FormatContext>
        auto format(const cc0::CompilationError &p, FormatContext &ctx) {
            return format_to(ctx.out(), "Line: {} Column: {} Error: {}", p.GetPos().first, p.GetPos().second,
                             p.GetCode());
        }
    };
}

namespace fmt {
    template<>
    struct formatter<cc0::Token> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

        template<typename FormatContext>
        auto format(const cc0::Token &p, FormatContext &ctx) {
            return format_to(ctx.out(),
                             "Line: {} Column: {} Type: {} Value: {}",
                             p.GetStartPos().first, p.GetStartPos().second, p.GetType(), p.GetValueString());
        }
    };

    template<>
    struct formatter<cc0::TokenType> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

        template<typename FormatContext>
        auto format(const cc0::TokenType &p, FormatContext &ctx) {
            std::string name;
            switch (p) {
                case cc0::NULL_TOKEN:
                    name = "NullToken";
                    break;
                case cc0::UNSIGNED_INTEGER:
                    name = "UnsignedInteger";
                    break;
                case cc0::IDENTIFIER:
                    name = "Identifier";
                    break;
                case cc0::BEGIN:
                    name = "Begin";
                    break;
                case cc0::END:
                    name = "End";
                    break;
                case cc0::VAR:
                    name = "Var";
                    break;
                case cc0::CONST:
                    name = "Const";
                    break;
                case cc0::PRINT:
                    name = "Print";
                    break;
                case cc0::PLUS_SIGN:
                    name = "PlusSign";
                    break;
                case cc0::MINUS_SIGN:
                    name = "MinusSign";
                    break;
                case cc0::MULTIPLICATION_SIGN:
                    name = "MultiplicationSign";
                    break;
                case cc0::DIVISION_SIGN:
                    name = "DivisionSign";
                    break;
                case cc0::EQUAL_SIGN:
                    name = "EqualSign";
                    break;
                case cc0::SEMICOLON:
                    name = "Semicolon";
                    break;
                case cc0::LEFT_BRACKET:
                    name = "LeftBracket";
                    break;
                case cc0::RIGHT_BRACKET:
                    name = "RightBracket";
                    break;
                case cc0::BACKSLASH:
                    name = "Backslash";
                    break;
                case cc0::STRING:
                    name = "String";
                    break;
                case cc0::CHAR_LIT:
                    name = "CharLiteral";
                    break;
                case cc0::ESCAPE_LIT:
                    name = "EscapeLiteral";
                    break;
                case cc0::INT:
                    name = "Integer";
                    break;
                case cc0::VOID:
                    name = "void";
                    break;
                case cc0::CHAR:
                    name = "CHAR";
                    break;
                case cc0::DOUBLE:
                    name = "DOUBLE";
                    break;
                case cc0::STRUCT:
                    name = "STRUCT";
                    break;
                case cc0::IF:
                    name = "if";
                    break;
                case cc0::ELSE:
                    name = "else";
                    break;
                case cc0::SWITCH:
                    name = "Switch";
                    break;
                case cc0::CASE:
                    name = "case";
                    break;
                case cc0::DEFAULT:
                    name = "default";
                    break;
                case cc0::WHILE:
                    name = "while";
                    break;
                case cc0::FOR:
                    name = "for";
                    break;
                case cc0::DO:
                    name = "do";
                    break;
                case cc0::RETURN:
                    name = "return";
                    break;
                case cc0::BREAK:
                    name = "BREAK";
                    break;
                case cc0::CONTINUE:
                    name = "CONTINUE";
                    break;
                case cc0::SCAN:
                    name = "SCAN";
                    break;
                case cc0::LEFT_BRACE:
                    name = "LEFT_BRACE";
                    break;
                case cc0::RIGHT_BRACE:
                    name = "RIGHT_BRACE";
                    break;
                case cc0::LEFT_PAREN:
                    name = "(";
                    break;
                case cc0::RIGHT_PAREN:
                    name = ")";
                    break;
                case cc0::COMMA:
                    name = ",";
                    break;
                case cc0::NOT:
                    name = "!";
                    break;
                case cc0::LESS:
                    name = "<";
                    break;
                case cc0::GREATER:
                    name = ">";
                    break;
                case cc0::LESS_EQ:
                    name = "<=";
                    break;
                case cc0::GREATER_EQ:
                    name = ">=";
                    break;
                case cc0::NOT_EQ:
                    name = "!=";
                    break;
                case cc0::EQUAL_EQ:
                    name = "==";
                    break;
                case cc0::INTEGER:
                    name = "int";
                    break;
                case cc0::FLOAT:
                    name = "double";
                    break;
                default:name = "undefine";
            }
            return format_to(ctx.out(), name);
        }
    };
}

namespace fmt {
    template<>
    struct formatter<cc0::Operation> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

        template<typename FormatContext>
        auto format(const cc0::Operation &p, FormatContext &ctx) {
            std::string name;
            switch (p){
                case cc0::NOP:
                    name = "nop";
                    break;
                case cc0::LOADA:
                    name = "loada";
                    break;
                case cc0::LOADC:
                    name = "loadc";
                    break;
                case cc0::IPUSH:
                    name = "ipush";
                    break;
                case cc0::BIPUSH:
                    name = "bipush";
                    break;
                case cc0::ILOAD:
                    name = "iload";
                    break;
                case cc0::DLOAD:
                    name = "dload";
                    break;
                case cc0::ISTORE:
                    name = "istore";
                    break;
                case cc0::CSTORE:
                    name = "cstore";
                    break;
                case cc0::DSTORE:
                    name = "dstore";
                    break;
                case cc0::NEW:
                    name = "new";
                    break;
                case cc0::SNEW:
                    name = "snew";
                    break;
                case cc0::POP:
                    name = "pop";
                    break;
                case cc0::DUP:
                    name = "dup";
                    break;
                case cc0::IADD:
                    name = "iadd";
                    break;
                case cc0::DADD:
                    name = "dadd";
                    break;
                case cc0::ISUB:
                    name = "isub";
                    break;
                case cc0::DSUB:
                    name = "dsub";
                    break;
                case cc0::IMUL:
                    name = "imul";
                    break;
                case cc0::DMUL:
                    name = "dmul";
                    break;
                case cc0::IDIV:
                    name = "idiv";
                    break;
                case cc0::DDIV:
                    name = "ddiv";
                    break;
                case cc0::INEG:
                    name = "ineg";
                    break;
                case cc0::DNEG:
                    name = "dneg";
                    break;
                case cc0::ICMP:
                    name = "icmp";
                    break;
                case cc0::DCMP:
                    name = "dcmp";
                    break;
                case cc0::I2D:
                    name = "i2d";
                    break;
                case cc0::I2C:
                    name = "i2c";
                    break;
                case cc0::D2I:
                    name = "d2i";
                    break;
                case cc0::JMP:
                    name = "jmp";
                    break;
                case cc0::JNE:
                    name = "jne";
                    break;
                case cc0::JE:
                    name = "je";
                    break;
                case cc0::JL:
                    name = "jl";
                    break;
                case cc0::JGE:
                    name = "jge";
                    break;
                case cc0::JG:
                    name = "jg";
                    break;
                case cc0::JLE:
                    name = "jle";
                    break;
                case cc0::CALL:
                    name = "call";
                    break;
                case cc0::RET:
                    name = "ret";
                    break;
                case cc0::IRET:
                    name = "iret";
                    break;
                case cc0::DRET:
                    name = "dret";
                    break;
                case cc0::IPRINT:
                    name = "iprint";
                    break;
                case cc0::DPRINT:
                    name = "dprint";
                    break;
                case cc0::CPRINT:
                    name = "cprint";
                    break;
                case cc0::SPRINT:
                    name = "sprint";
                    break;
                case cc0::PRINTL:
                    name = "printl";
                    break;
                case cc0::ISCAN:
                    name = "iscan";
                    break;
                case cc0::CSCAN:
                    name = "cscan";
                    break;
                case cc0::DSCAN:
                    name = "dscan";
                    break;
                default:
                    return format_to(ctx.out(),"error");
            }
            return format_to(ctx.out(),name);
        }
    };

    template<>
    struct formatter<cc0::Instruction> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

        template<typename FormatContext>
        auto format(const cc0::Instruction &p, FormatContext &ctx) {
            switch (p.GetOperation()) {
                // 0
                case cc0::NOP:
                case cc0::POP:
                case cc0::DUP:
                case cc0::NEW:
                case cc0::TLOAD:
                case cc0::ILOAD:
                case cc0::DLOAD:
                case cc0::TADD:
                case cc0::IADD:
                case cc0::DADD:
                case cc0::TSUB:
                case cc0::DSUB:
                case cc0::ISUB:
                case cc0::TMUL:
                case cc0::IMUL:
                case cc0::DMUL:
                case cc0::TDIV:
                case cc0::IDIV:
                case cc0::DDIV:
                case cc0::TNEG:
                case cc0::INEG:
                case cc0::DNEG:
                case cc0::TCMP:
                case cc0::ICMP:
                case cc0::DCMP:
                case cc0::TALOAD:
                case cc0::TSTORE:
                case cc0::ISTORE:
                case cc0::CSTORE:
                case cc0::DSTORE:
                case cc0::TASTORE:
                case cc0::I2D:
                case cc0::I2C:
                case cc0::D2I:
                case cc0::RET:
                case cc0::TRET:
                case cc0::IRET:
                case cc0::DRET:
                case cc0::TPRINT:
                case cc0::IPRINT:
                case cc0::DPRINT:
                case cc0::SPRINT:
                case cc0::CPRINT:
                case cc0::PRINTL:
                case cc0::TSCAN:
                case cc0::ISCAN:
                case cc0::CSCAN:
                case cc0::DSCAN:
                    return format_to(ctx.out(), "{}", p.GetOperation());
                // 1
                case cc0::LOADC:
                case cc0::BIPUSH:
                case cc0::IPUSH:
                case cc0::JMP:
                case cc0::SNEW:
                case cc0::JNE:
                case cc0::JE:
                case cc0::JL:
                case cc0::JGE:
                case cc0::JG:
                case cc0::JLE:
                case cc0::CALL:
                    return format_to(ctx.out(), "{} {}", p.GetOperation(), p.GetX());
                case cc0::LOADA:
                    return format_to(ctx.out(), "{} {}, {}", p.GetOperation(), p.GetX(), p.getY());

            }
            return format_to(ctx.out(), "NOP");
        }
    };

    template<>
    struct formatter<cc0::CONST_TYPE> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

        template<typename FormatContext>
        auto format(const cc0::CONST_TYPE &p, FormatContext &ctx) {
            std::string name;
            switch (p) {
                case cc0::I:
                    name = "I";
                    break;
                case cc0::S:
                    name = "S";
                    break;
                case cc0::D:
                    name = "D";
                    break;
                default:
                    name = "";
            }
            return format_to(ctx.out(), name);
        }
    };


    template<>
    struct formatter<cc0::Constants> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

        template<typename FormatContext>
        auto format(const cc0::Constants &p, FormatContext &ctx) {
            switch (p.GetConstType()) {
                case cc0::S:
                    return format_to(ctx.out(), "{} {} \"{}\"", p.GetIndex(), p.GetConstType(), p.Get());
                case cc0::I:
                    return format_to(ctx.out(), "{} {} {}", p.GetIndex(), p.GetConstType(), p.Get());
                case cc0::D:
                    return format_to(ctx.out(), "{} {} {}", p.GetIndex(), p.GetConstType(), p.Get());
                default:
                    return format_to(ctx.out(),"");

            }
        }
    };

    template<>
    struct formatter<cc0::Function> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

        template<typename FormatContext>
        auto format(const cc0::Function &p, FormatContext &ctx) {
            return format_to(ctx.out(), "{} {} {} {}", p.getIndex(), p.getNameIndex(), p.getParamsSize(), p.getLevel());
        }
    };
}