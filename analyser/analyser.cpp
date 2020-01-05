#include "analyser.h"

#include <climits>

namespace cc0 {
    std::pair<std::pair<std::vector<Constants>, std::vector<Function>>, std::optional<CompilationError>>
    Analyser::Analyse() {
        auto err = analyseProgram();
        if (err.has_value())
            return std::make_pair(std::make_pair(std::vector<Constants>(), std::vector<Function>()), err);
        else
            return std::make_pair(std::make_pair(_constants, _functions), std::optional<CompilationError>());
    }

    // <C0-program> ::=
    //    {<variable-declaration>}{<function-definition>}
    std::optional<CompilationError> Analyser::analyseProgram() {
        auto varErr = analyseVariableDeclaration();
        if (varErr.has_value())
            return varErr;

        _isStart = false;
        auto err = analyseFunction();
        if (err.has_value())
            return err;

        if (!isFunction("main"))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedMain);


        return {};
    }

    // {<function-definition>} ::=
    //    <type-specifier><identifier><parameter-clause><compound-statement>
    std::optional<CompilationError> Analyser::analyseFunction() {
        // function-definition可能有一个或者多个
        while (true) {
            // 预读
            auto next = nextToken();
            if (!next.has_value())
                return {};

            CONST_TYPE type;
            if (next.value().GetType() == TokenType::INT) {
                type = I;
            } else if (next.value().GetType() == TokenType::DOUBLE) {
                type = D;
            } else if (next.value().GetType() == TokenType::CHAR) {
                type = C;
            } else if (next.value().GetType() == TokenType::VOID) {
                type = V;
            } else {
                unreadToken();
                return {};
            }

            //pre read
            next = nextToken();
            if (!next.has_value() || next.value().GetType() != IDENTIFIER)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionDeclared);

            auto name = next.value().GetValueString();

            _constants.emplace_back(S, name, _nextConstIndex++);
            if (_current_function.empty())
                _functions.emplace_back(_nextConstIndex - 1, name, _nextFunIndex++, 1, true, type);
            else
                _functions.emplace_back(_nextConstIndex - 1, name, _nextFunIndex++,
                                        _current_function.back().getLevel() + 1,
                                        true, type);

            _current_function.push_back(_functions.back());

            auto err = analyseParameterClause();
            if (err.has_value())
                return err;

            err = analyseCompoundStatement(false);
            if (err.has_value())
                return err;

            addInstruction(V, RET, 0);
            _functions.at(_current_function.back().getIndex()) = _current_function.back();
            _current_function.erase(_current_function.end());
        }
        return {};
    }

    std::optional<CompilationError> Analyser::analyseParameterClause() {
        auto next = nextToken();
        if (!next.has_value() || next.value().GetType() != LEFT_PAREN)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionDeclared);

        next = nextToken();
        if (next.value().GetType() == RIGHT_PAREN)
            return {};
        try {
            while (next.value().GetType() != RIGHT_PAREN) {
                if (next.value().GetType() != COMMA) unreadToken();

                next = nextToken();
                if (!next.has_value())
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);


                bool isConst = false;
                if (next.value().GetType() == TokenType::CONST) {
                    isConst = true;
                    next = nextToken();

                }

                CONST_TYPE type;
                if (next.value().GetType() == TokenType::INT) {
                    type = I;
                } else if (next.value().GetType() == TokenType::DOUBLE) {
                    type = D;
                } else if (next.value().GetType() == TokenType::CHAR) {
                    type = C;
                } else if (isConst) { // is const but has not type
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedVarType);
                } else {
                    unreadToken();
                    return {};
                }

                next = nextToken();
                if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
                auto dec = isDeclared(next.value().GetValueString());
                if (dec.first && dec.second == 0)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

                auto iToken = next.value();
                if (type == D) {
                    if (isConst) addDoubleConst(iToken);
                    else
                        addDouble(iToken);
                } else {
                    if (isConst) addLocalConstant(iToken, type);
                    else
                        addVariable(iToken, type);
                }

                _current_function.back().paramsAdd(type);

                next = nextToken();  //pre )
            }
        } catch (ErrorCode) {
            return std::make_optional<CompilationError>(_current_pos, ErrFunctionDeclared);
        }

        return {};
    }

    std::optional<CompilationError> Analyser::analyseCompoundStatement(bool uplevel) {

        if (uplevel)
            _current_function.back().addLevel();

        auto next = nextToken();
        if (!next.has_value() || next.value().GetType() != LEFT_BRACE)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionDeclared);


        // <变量声明>
        auto varErr = analyseVariableDeclaration();
        if (varErr.has_value())
            return varErr;
        _functions.at(_current_function.back().getIndex()) = _current_function.back();

        // <语句序列>
        auto staErr = analyseStatementSequence(true);
        if (staErr.has_value())
            return staErr;

        next = nextToken();
        if (!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);
        if (next.value().GetType() != RIGHT_BRACE)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidInput);

        _current_function.back().downLevel();
        return {};
    }

    // {<variable-declaration>} ::=
    //    [<const-qualifier>]<type-specifier><init-declarator-list>';'
    std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
        // 变量声明语句可能有一个或者多个
        while (true) {
            // 预读
            auto next = nextToken();
            if (!next.has_value())
                return {};


            bool isConst = false;
            if (next.value().GetType() == TokenType::CONST) {
                isConst = true;
                next = nextToken();
            }

            CONST_TYPE type = V;
            if (next.value().GetType() == TokenType::INT) {
                type = I;
            } else if (next.value().GetType() == TokenType::DOUBLE) {
                type = D;
            } else if (next.value().GetType() == TokenType::CHAR) {
                type = C;
            } else if (isConst) { // is const but has not type
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedVarType);
            } else {
                unreadToken();
                return {};
            }
            // 预读 两个 如果是 类型已读 + ID + （ ,转 function
            auto i = nextToken();
            auto l = nextToken();
            unreadToken();
            unreadToken();
            if (!i.has_value() || !l.has_value())
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
            if (i.value().GetType() == TokenType::IDENTIFIER && l.value().GetType() == TokenType::LEFT_PAREN)// ( 转函数定义
            {
                unreadToken();
                return {};
            }

            auto err = analyseInitDeclaratorList(type, isConst);
            if (err.has_value())
                return err;

            //;
            next = nextToken();
            if (next.value().GetType() != TokenType::SEMICOLON)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
        }

        return {};
    }

    std::optional<CompilationError> Analyser::analyseInitDeclaratorList(CONST_TYPE type, bool isConst) {
        // <标识符>
        auto next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
        auto dec = isDeclared(next.value().GetValueString());
        if (dec.first && dec.second == 0)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

        auto name = next.value().GetValueString();
        auto iToken = next.value();
        // 变量可能没有初始化，仍然需要一次预读
        next = nextToken();

        if (!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrNoSemicolon);

        // '=' ，先为无=
        if (next.value().GetType() != TokenType::EQUAL_SIGN) {
            if (isConst)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantNeedValue);

            //加入变量表
            if (type == D) {
                addUninitializedDouble(iToken);
            } else
                addUninitializedVariable(iToken, type);

            //在栈上先占个位
            if (_isStart) _start.emplace_back(Operation::IPUSH, 0);
            else
                addInstruction(I, IPUSH, 0);
            if (type == D) {
                if (_isStart) _start.emplace_back(Operation::IPUSH, 0);
                else
                    addInstruction(I, IPUSH, 0);
                addInstruction(V, I2D, 0);
            }
        }
            // 有 =
        else {
            // '<表达式>'
            if (type == D) {
                if (isConst) addDoubleConst(iToken);
                else
                    addDouble(iToken);
            } else {
                if (isConst) addLocalConstant(iToken, type);
                else
                    addVariable(iToken, type);
            }

            auto err = analyseExpression(type);
            if (err.has_value())
                return err;

            next = nextToken();
        }

        //,
        if (!next.has_value())
            return {};
        if (next.value().GetType() == COMMA) {
            auto err = analyseInitDeclaratorList(type, isConst);
            if (err.has_value())
                return err;
        } else unreadToken();
        return {};
    }

    // <语句序列> ::= {<语句>}
    // <语句> :: = <赋值语句> | <输出语句> | <空语句>
    // <赋值语句> :: = <标识符>'='<表达式>';'
    // <输出语句> :: = 'print' '(' <表达式> ')' ';'
    // <空语句> :: = ';'
    // <statement> ::=
    //     <compound-statement>
    //    |<condition-statement>
    //    |<loop-statement>
    //    |<jump-statement>
    //    |<print-statement>
    //    |<scan-statement>
    //    |<assignment-expression>';'
    //    |<function-call>';'
    //    |';'
    std::optional<CompilationError> Analyser::analyseStatementSequence(bool flag) {
        do {
            // 预读
            auto next = nextToken();
            if (!next.has_value())
                return {};

            if (next.value().GetType() == TokenType::LEFT_BRACE) {
                unreadToken();
                auto err = analyseCompoundStatement(true);
                if (err.has_value())
                    return err;
                continue;
            }

            unreadToken();
            auto tp = next.value().GetType();
            if (tp == IF || tp == SWITCH) {
                auto err = analyseConditionStatement();
                if (err.has_value())
                    return err;
            } else if (tp == WHILE || tp == DO || tp == FOR) {
                auto err = analyseLoopStatement();
                if (err.has_value())
                    return err;
            } else if (tp == BREAK || tp == CONTINUE || tp == RETURN) {
                auto err = analyseJumpStatement();
                if (err.has_value())
                    return err;
            } else if (tp == PRINT) {
                auto err = analysePrintStatement();
                if (err.has_value())
                    return err;
            } else if (tp == SCAN) {
                auto err = analyseScanStatement();
                if (err.has_value())
                    return err;
            } else if (tp == IDENTIFIER) {
                nextToken();
                next = nextToken();
                unreadToken();
                unreadToken();
                if (next.value().GetType() == LEFT_PAREN) {
                    auto err = analyseFunctionCall(I);
                    if (err.has_value())
                        return err;
                } else {
                    auto err = analyseAssignmentExpression();
                    if (err.has_value())
                        return err;
                }
                next = nextToken();
                if (next.value().GetType() != SEMICOLON)
                    return std::make_optional<CompilationError>(_current_pos, ErrNoSemicolon);
            } else {
                if (tp == SEMICOLON) nextToken();
                if (!flag && tp != SEMICOLON)
                    return std::make_optional<CompilationError>(_current_pos, ErrNoSemicolon);
                return {};
            }
        } while (flag);
        return {};
    }

    // <expression> ::=
    //    <additive-expression>
    // <additive-expression> ::=
    //     <multiplicative-expression>{<additive-operator><multiplicative-expression>}
    std::optional<CompilationError> Analyser::analyseExpression(CONST_TYPE type) {

        if (_expression_level.size() > 10)_expression_level.clear();
        _expression_level.emplace_back(C);
        // <multiplicative-expression>
        auto err = analyseMultiExpression(D);
        if (err.has_value())
            return err;

        // {<加法型运算符><项>} {<additive-operator><multiplicative-expression>}
        while (true) {
            // 预读
            auto next = nextToken();
            if (!next.has_value())
                break;
            auto tokenType = next.value().GetType();
            if (tokenType != TokenType::PLUS_SIGN && tokenType != TokenType::MINUS_SIGN) {
                unreadToken();
                break;
            }

            // <multiplicative-expression>
            err = analyseMultiExpression(D);
            if (err.has_value())
                return err;

            _expression_level.emplace_back(C);

            // 根据结果生成指令
            if (tokenType == TokenType::PLUS_SIGN)
                addInstruction(D, TADD, 0);
            else addInstruction(D, TSUB, 0);
        }

        if (type != D && _expression_level.back() != D)
            addInstruction(V, D2I, 0);


        return {};
    }


    // <赋值语句> ::= <标识符>'='<表达式>';'
    // <assignment-expression> ::=
    //    <identifier><assignment-operator><expression>
    std::optional<CompilationError> Analyser::analyseAssignmentExpression() {
        // 这里除了语法分析以外还要留意
        auto next = nextToken();
        if (!next.has_value())
            return {};
        // 是标识符
        if (next.value().GetType() != TokenType::IDENTIFIER) {
            unreadToken();
            return {};
        }
        auto iToken = next.value();
        auto name = next.value().GetValueString();
        // 标识符声明过吗？
        if (!isDeclared(name).first)
            return std::make_optional<CompilationError>(_current_pos, ErrNotDeclared);

        // 标识符是常量吗？
        if (isConstant(name))
            return std::make_optional<CompilationError>(_current_pos, ErrAssignToConstant);



        // '='
        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantNeedValue);


        auto it = getVarInfo(name);
        auto ty = it.second.getType();
//        std::cout << it.first << "  " << it.second;
        addInstruction(N, LOADA, it.first, it.second.getIndex());


        auto err = analyseExpression(ty);
        if (err.has_value())
            return err;
        if (isUninitializedVariable(name))
            assignVar(name);
        addInstruction(ty, TSTORE, 0);

        return {};
    }


    // <项> :: = <因子>{ <乘法型运算符><因子> }
    // <multiplicative-expression> ::=
    //     <cast-expression>{<multiplicative-operator><cast-expression>}
    std::optional<CompilationError> Analyser::analyseMultiExpression(CONST_TYPE type) {
        // <cast-expression>
        auto err = analyseCastExpression(type);
        if (err.has_value())
            return err;

        // {<乘法型运算符><因子>} {<multiplicative-operator><cast-expression>}
        while (true) {
            // 预读
            auto next = nextToken();
            if (!next.has_value())
                return {};
            auto tokenType = next.value().GetType();
            if (tokenType != TokenType::MULTIPLICATION_SIGN && tokenType != TokenType::DIVISION_SIGN) {
                unreadToken();
                return {};
            }

            // <cast-expression>
            err = analyseCastExpression(type);
            if (err.has_value())
                return err;

            // 根据结果生成指令
            // todo tokenType
            if (tokenType == TokenType::MULTIPLICATION_SIGN)
                addInstruction(D, TMUL, 0);
            else addInstruction(D, TDIV, 0);
        }

        return {};
    }

    // <cast-表达式> ::=
    //    {'('<类型-说明符号>')'}<正负号-表达式>
    // <cast-expression> ::=
    //    {'('<type-specifier>')'}<unary-expression>
    std::optional<CompilationError> Analyser::analyseCastExpression(CONST_TYPE type) {
        // [(类型-说明符号)]
        auto next = nextToken();
        if (!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

        bool nocast = false;
        if (next.value().GetType() == TokenType::LEFT_PAREN) {
            next = nextToken();
            if (!next.has_value())
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

            CONST_TYPE renameType = V;
            switch (next.value().GetType()) {
                case DOUBLE:
                    renameType = D;
                    break;
                case CHAR:
                    renameType = C;
                    break;
                case INT:
                    renameType = I;
                    break;
                case VOID:
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrViod);
                default:
                    unreadToken();
                    unreadToken();
                    nocast = true;
            }

            if (!nocast) {
                next = nextToken();
                if (!next.has_value() || next.value().GetType() != RIGHT_PAREN)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);
            }


        } else {
            unreadToken();
        }
        auto err = analyseUnaryExpression(type);
        if (err.has_value())
            return err;

        return {};
    }

    // <unary-expression> ::=
    //    [<unary-operator '+' | '-' >]<primary-expression>
    // <primary-expression> ::=
    //     '('<expression>')'
    //    |<identifier>
    //    |<integer-literal>
    //    |<char-literal>
    //    |<floating-literal>
    //    |<function-call>
    std::optional<CompilationError> Analyser::analyseUnaryExpression(CONST_TYPE type) {
        // [<符号>]
        auto next = nextToken();
        auto prefix = 1;
        if (!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
        if (next.value().GetType() == TokenType::PLUS_SIGN)
            prefix = 1;
        else if (next.value().GetType() == TokenType::MINUS_SIGN) {
            prefix = -1;
        } else
            unreadToken();

        std::optional<CompilationError> err;

        // 预读
        next = nextToken();
        if (!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

        int32_t val;
        switch (next.value().GetType()) {
            // 这里和 <语句序列> 类似，需要根据预读结果调用不同的子程序
            // 但是要注意 default 返回的是一个编译错误
            case LEFT_PAREN:
                err = analyseExpression(type);
                if (err.has_value()) return err;
                next = nextToken();
                if (!next.has_value() || next.value().GetType() != RIGHT_PAREN)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);
                break;
            case FLOAT:
                _constants.emplace_back(D, next.value().GetValueString(), _nextConstIndex++);
                addInstruction(V, LOADC, _nextConstIndex - 1);
                _expression_level.back() = D;
                break;
            case CHAR:
                val = std::any_cast<int32_t>(next.value().GetValue());
                addInstruction(C, BIPUSH, val);
                addInstruction(V, I2D, 0);
                break;
            case INTEGER:
                try {
                    val = std::any_cast<int32_t>(next.value().GetValue());
                    if (INT32_MIN <= val && val <= INT32_MAX)
                        addInstruction(type, IPUSH, (int32_t) val);
                    addInstruction(V, I2D, 0);
                    if (_expression_level.back() == C)_expression_level.back() = I;
                } catch (ErrorCode) {
                    return std::make_optional<CompilationError>(_current_pos, ErrIntegerOverflow);
                }
                break;
            case IDENTIFIER: {
                // 预读便于调用
                Token iToken = next.value();
                next = nextToken();
                // 不管如何，都把预读的写回去
                unreadToken();
                unreadToken();
                auto tp = type > _expression_level.back() ? type : _expression_level.back();
                if (next.value().GetType() == LEFT_PAREN)//<function-call>
                {
                    err = analyseFunctionCall(tp);
                    if (tp == D) {
                        addInstruction(V, I2D, 0);
                    }
                    if (err.has_value()) return err;
                } else { //<identifier>
                    err = analyseIdentifier(tp);
                    if (err.has_value()) return err;
                }
                break;
            }
            default:
                unreadToken();
                return {};
//                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
        }

        // 取负
        if (prefix == -1)
            addInstruction(type, TNEG, 0);
        return {};
    }

    std::optional<CompilationError> Analyser::analyseIdentifier(CONST_TYPE type) {
        auto next = nextToken();
        auto name = next.value().GetValueString();
        if (!isDeclared(next.value().GetValueString()).first)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
        if (isUninitializedVariable(name))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);

        auto it = getVarInfo(name);
        auto info = it.second;
        if (info.getType() == D)
            _expression_level.back() = info.getType();

        addInstruction(N, LOADA, it.first, it.second.getIndex());
        addInstruction(info.getType(), TLOAD, 0);

        if (info.getType() != D && type == D)
            addInstruction(V, I2D, 0);


        return {};
    }

    std::optional<CompilationError> Analyser::analyseFunctionCall(CONST_TYPE type) {
        auto next = nextToken();
        auto name = next.value().GetValueString();
        if (!isFunction(name))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);

        nextToken();  // (
        // pre read
        next = nextToken();
        auto reTy = getFunction(name).getReType();
        auto pa = getFunction(name).getParams();
        if (next.value().GetType() == RIGHT_PAREN)  // ) ,无参数
        {
            addInstruction(V, CALL, getFunctionIndex(name));
        } else {
            try {
                int32_t len = 0;
                while (next.value().GetType() != RIGHT_PAREN) {
                    if (next.value().GetType() != COMMA) unreadToken();
                    auto err = analyseExpression(pa[len]);
                    if (err.has_value()) return err;
                    next = nextToken();
                    len++;
                    if (len > getFunction(name).getParamNum()) // 参数长度不匹配
                        return std::make_optional<CompilationError>(_current_pos, ErrFunctionParams);
                }


                addInstruction(V, CALL, getFunctionIndex(name));

            } catch (ErrorCode) {
                return std::make_optional<CompilationError>(_current_pos, ErrNoRightBracket);
            }

        }

        if (reTy == D)
            _expression_level.back() = D;


        return {};
    }

    std::optional<CompilationError> Analyser::analyseConditionStatement() {
        auto next = nextToken();
        if (next.value().GetType() == IF) {

            next = nextToken();
            if (!next.has_value() || next.value().GetType() != LEFT_PAREN)
                return std::make_optional<CompilationError>(_current_pos, ErrConditionStatement);


            auto err = analyseCondition();
            if (err.has_value()) return err;

            auto jmpIndex = _current_function.back().getInsLen() - 1;


            next = nextToken();
            if (!next.has_value() || next.value().GetType() != RIGHT_PAREN)
                return std::make_optional<CompilationError>(_current_pos, ErrConditionStatement);

            err = analyseStatementSequence(false);
            if (err.has_value()) return err;

            auto elseIndex = _current_function.back().getInsLen();
            _current_function.back().changeJumpX(jmpIndex, elseIndex);


            next = nextToken();
            if (!next.has_value() || next.value().GetType() != ELSE) {
                unreadToken();
                return {};
            }

            err = analyseStatementSequence(false);
            if (err.has_value()) return err;

        } else { // todo switch
            unreadToken();
            return {};
        }
        return {};
    }

    std::optional<CompilationError> Analyser::analyseLoopStatement() {
        auto next = nextToken();

        if (next.value().GetType() == WHILE) {

            next = nextToken();
            if (!next.has_value() || next.value().GetType() != LEFT_PAREN)
                return std::make_optional<CompilationError>(_current_pos, ErrConditionStatement);

            int32_t startIndex = _current_function.back().getInsLen();   // while 开始地址

            auto err = analyseCondition();  // 最后一定是不满足条件跳转语句
            if (err.has_value()) return err;

            auto jmpIndex = _current_function.back().getInsLen() - 1; // jmp 指令的地址

            next = nextToken();
            if (!next.has_value() || next.value().GetType() != RIGHT_PAREN)
                return std::make_optional<CompilationError>(_current_pos, ErrConditionStatement);


            err = analyseStatementSequence(false);
            if (err.has_value()) return err;

            addInstruction(N, JMP, startIndex); // 无条件跳转while头

            int32_t endIndex = _current_function.back().getInsLen();  // while 结束地址
            _current_function.back().changeJumpX(jmpIndex, endIndex);


        } else { // todo loop for do any
            unreadToken();
            return {};
        }

        return {};
    }

    std::optional<CompilationError> Analyser::analyseJumpStatement() {
        auto next = nextToken();
        if (!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrJumpStatement);
        auto tp = next.value().GetType();
        if (tp == BREAK) {
            // TODO BREAK
            unreadToken();
            return {};
        } else if (tp == CONTINUE) {
            // TODO CONTINUE
            unreadToken();
            return {};
        } else if (tp == RETURN) {
            next = nextToken();
            if (!next.has_value())
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrJumpStatement);
            unreadToken();

            auto ty = _current_function.back().getReType();
            if (ty != V) {
                auto err = analyseExpression(ty);
                if (err.has_value()) return err;
//                if (ty != D)
//                    addInstruction(V, D2I, 0);
                addInstruction(ty, TRET, 0);
            } else {
                addInstruction(V, RET, 0);
            }
        } else {
            unreadToken();
            return {};
        }

        next = nextToken();
        if (!next.has_value() || next.value().GetType() != SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
        return {};
    }

// <输出语句> ::= 'print' '(' <表达式> ')' ';'
// <print-statement> ::= 'print' '(' [<printable-list>] ')' ';'
//  <printable-list>  ::= <printable> {',' <printable>}
//  <printable> ::= <expression>
    std::optional<CompilationError> Analyser::analysePrintStatement() {
        // 如果之前 <语句序列> 的实现正确，这里第一个 next 一定是 TokenType::PRINT
        auto next = nextToken();

        // '('
        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::LEFT_PAREN)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);



        // ')'
        next = nextToken();
        if (!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);

        try {
            while (next.value().GetType() != RIGHT_PAREN) {
                if (next.value().GetType() != COMMA) unreadToken();
                else {
                    addInstruction(I, BIPUSH, 0);
                    addInstruction(N, CPRINT, 0);
                }

                next = nextToken();
                unreadToken();
                if (!next.has_value())
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);

                // <表达式>
                auto err = analyseExpression(N);
                if (err.has_value())
                    return err;

                auto ty = _expression_level.back();

                addInstruction(ty, TPRINT, 0);

                next = nextToken();  //pre )
            }
            addInstruction(V, PRINTL, 0);
        } catch (ErrorCode) {
            return std::make_optional<CompilationError>(_current_pos, ErrFunctionDeclared);
        }

        // ')'
        if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_PAREN)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);


        // ';'
        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

        return {};
    }

    std::optional<CompilationError> Analyser::analyseScanStatement() {
        auto next = nextToken();

        // '('
        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::LEFT_PAREN)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);

        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrScanStatement);

        auto name = next.value().GetValueString();

        if (!isDeclared(name).first)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
        if (isConstant(name))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);

        // todo type
        auto it = getVarInfo(name);
        auto ty = it.second.getType();
        addInstruction(N, LOADA, it.first, it.second.getIndex());
        addInstruction(ty, TSCAN, 0);
        addInstruction(ty, TSTORE, 0);
        if (isUninitializedVariable(name)) {
            assignVar(name);
        }




        // ')'
        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_PAREN)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrScanStatement);



        // ';'
        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

        return {};


    }

    std::optional<CompilationError> Analyser::analyseCondition() {

        auto next = nextToken();
        if (!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConditionStatement);
        unreadToken();

        auto err = analyseExpression(D);
        if (err.has_value()) return err;

        // re
        next = nextToken();
        if (!next.has_value()) {
            unreadToken();
            return {};
        }
        auto tp = next.value().GetType();
        if (tp == LESS) {
            err = analyseExpression(D);
            if (err.has_value()) return err;
            addInstruction(D, TCMP, 0);  // > 1 = 0
            addInstruction(D, JGE, 0); // >=0 跳转
        } else if (tp == LESS_EQ) {
            err = analyseExpression(D);
            if (err.has_value()) return err;
            addInstruction(D, TCMP, 0);  // > 1
            addInstruction(D, JG, 0); // > 0 跳转
        } else if (tp == GREATER) {
            err = analyseExpression(D);
            if (err.has_value()) return err;
            addInstruction(D, TCMP, 0);  // < -1 = 0
            addInstruction(D, JLE, 0); // <=0 跳转
        } else if (tp == GREATER_EQ) {
            err = analyseExpression(D);
            if (err.has_value()) return err;
            addInstruction(D, TCMP, 0);  // < -1
            addInstruction(D, JL, 0); // < 0 跳转
        } else if (tp == NOT_EQ) {
            err = analyseExpression(D);
            if (err.has_value()) return err;
            addInstruction(D, TCMP, 0);  //  = 0
            addInstruction(D, JE, 0); // ==0 跳转

        } else if (tp == EQUAL_EQ) {
            err = analyseExpression(D);
            if (err.has_value()) return err;
            addInstruction(D, TCMP, 0);//  > 0 <0
            addInstruction(D, JNE, 0); // !=0 跳转
        } else {
            addInstruction(D, JE, 0);
            unreadToken();
            return {};
        }
        return {};
    }


    std::optional<Token> Analyser::nextToken() {
        if (_offset == _tokens.size())
            return {};
        // 考虑到 _tokens[0..._offset-1] 已经被分析过了
        // 所以我们选择 _tokens[0..._offset-1] 的 EndPos 作为当前位置
        _current_pos = _tokens[_offset].GetEndPos();
        return _tokens[_offset++];
    }

    void Analyser::unreadToken() {
        if (_offset == 0)
            DieAndPrint("analyser unreads token from the begining.");
        _current_pos = _tokens[_offset - 1].GetEndPos();
        _offset--;
    }

    bool Analyser::isUninitializedVariable(const std::string &s) {
        auto var = getVarInfo(s);
        return var.second.isIsUnInit();
    }

    bool Analyser::isInitializedVariable(const std::string &s) {
        auto var = getVarInfo(s);
        return !var.second.isIsUnInit();

    }

    bool Analyser::isConstant(const std::string &s) {
        auto var = getVarInfo(s);
        return var.second.isIsConst();
    }

    // 1 为全局 0 为自身
    std::pair<bool, int> Analyser::isDeclared(const std::string &s) {
        // todo char double
        if (_isStart)
            return std::make_pair(_g_vars.isDeclared(s), 1);
        bool child = _current_function.back().isDeclared(s);
        if (child) return std::make_pair(child, 0);
        else
            return std::make_pair(_g_vars.isDeclared(s), 1);
    }

    void Analyser::addUninitializedVariable(const Token &tk, CONST_TYPE type) {
        if (_isStart) _g_vars.addVar(tk.GetValueString(), type, _nextTokenIndex++, true, false);
        else _current_function.back().addUninitializedVariable(tk, type);
    }

    void Analyser::addVariable(const Token &tk, CONST_TYPE type) {
        if (_isStart) _g_vars.addVar(tk.GetValueString(), type, _nextTokenIndex++, false, false);
        else _current_function.back().addVariable(tk, type);
    }

    void Analyser::addLocalConstant(const Token &tk, CONST_TYPE type) {
        if (_isStart) _g_vars.addVar(tk.GetValueString(), type, _nextTokenIndex++, false, true);
        else _current_function.back().addLocalConstant(tk, type);
    }

    // and double
    void Analyser::addDouble(const cc0::Token &tk) {
//        _constants.emplace_back(D, tk.GetValueString(), _nextConstIndex++);

        if (_isStart) _g_vars.addVar(tk.GetValueString(), D, _nextTokenIndex++, false, false);
        else _current_function.back().addDouble(tk, _nextTokenIndex++);
        _nextTokenIndex++;
    }

    // 添加double未初始化变量
    void Analyser::addUninitializedDouble(const Token &tk) {
//        _constants.emplace_back(D, tk.GetValueString(), _nextConstIndex++);
        if (_isStart)_g_vars.addVar(tk.GetValueString(), D, _nextTokenIndex++, true, false);
        else _current_function.back().addUninitializedDouble(tk, _nextTokenIndex++);
        _nextTokenIndex++;
    }


    // 添加 double 常量
    void Analyser::addDoubleConst(const Token &tk) {
//        _constants.emplace_back(D, tk.GetValueString(), _nextConstIndex++);
        if (_isStart)_g_vars.addVar(tk.GetValueString(), D, _nextTokenIndex++, false, true);
        else _current_function.back().addDoubleConst(tk, _nextTokenIndex++);
        _nextTokenIndex++;
    }

    void Analyser::addInstruction(CONST_TYPE type, Operation op, int32_t x) {
        Operation realOp = NOP;
        switch (op) {
            case TADD:
                if (type == I || type == C)
                    realOp = IADD;
                else realOp = DADD;
                break;
            case TMUL:
                if (type == I || type == C)
                    realOp = IMUL;
                else realOp = DMUL;
                break;
            case TDIV:
                if (type == I || type == C)
                    realOp = IDIV;
                else realOp = DDIV;
                break;
            case TSUB:
                if (type == I || type == C)
                    realOp = ISUB;
                else realOp = DSUB;
                break;
            case TNEG:
                if (type == I || type == C)
                    realOp = INEG;
                else realOp = DNEG;
                break;
            case TRET:
                if (type == I || type == C)
                    realOp = IRET;
                else realOp = DRET;
                break;
            case TPRINT:
                if (type == I || type == C)
                    realOp = IPRINT;
                else realOp = DPRINT;
                break;
            case TLOAD:
                if (type == I || type == C)
                    realOp = ILOAD;
                else realOp = DLOAD;
                break;
            case IPUSH:
                realOp = IPUSH;
                break;
            case TCMP:
                if (type == I || type == C)
                    realOp = ICMP;
                else realOp = DCMP;
                break;
            case TSCAN:
                if (type == I)
                    realOp = ISCAN;
                else if (type == C)
                    realOp = CSCAN;
                else realOp = DSCAN;
                break;
            case TSTORE:
                if (type == I)
                    realOp = ISTORE;
                else if (type == C)
                    realOp = CSTORE;
                else realOp = DSTORE;
                break;
            default:
                realOp = op;
        }

        if (_isStart) {
            _start.emplace_back(realOp, x);
        } else {
            _current_function.back().addInstruction(realOp, x);
        }

    }

    void Analyser::addInstruction(CONST_TYPE type, Operation op, int32_t x, int32_t y) {
        if (_isStart) {
            if (type == I) _start.emplace_back(op, x, y);
        } else {
            _current_function.back().addInstruction(op, x, y);
        }

    }

    bool Analyser::canReDefine(const std::string &s) {
        if (_isStart) {
            return false;
        } else {
            _current_function.back().canReDefine(s);
        }
    }

    // level index
    std::pair<int32_t, Var> Analyser::getVarInfo(const std::string &s) {
        // todo getVarInfo
        if (_isStart) {
            if (_g_vars.isDeclared(s))
                return std::make_pair(1, _g_vars.getVar(s));
        } else {
            auto re = isDeclared(s);
            if (re.second == 0)return std::make_pair(0, _current_function.back().getVarInfo(s));
            else
                return std::make_pair(1, _g_vars.getVar(s));
        }
        return std::make_pair(1, _g_vars.getVar(s));
    }


    bool Analyser::isFunction(const std::string &s) {
        for (auto &it : _functions) {
            if (it.getName() == s) return true;
        }
        return false;
    }

    int32_t Analyser::getFunctionIndex(const std::string &s) {
        for (auto &it : _functions) {
            if (it.getName() == s) return it.getIndex();
        }
        return {};
    }

    Function Analyser::getFunction(const std::string &s) {
        for (auto &it : _functions) {
            if (it.getName() == s) return it;
        }
        return {};
    }

    void Analyser::assignVar(const std::string &s) {
        if (_isStart) {
            _g_vars.assignVar(s);
        } else {
            _current_function.back().assignVar(s);
        }
    }
}