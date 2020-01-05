#pragma once

#include "type/instruction.h"
#include "type/constans.h"
#include "type/funciton.h"
#include "tokenizer/token.h"

#include <vector>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef> // for std::size_t

namespace cc0 {

	class Analyser final {
	private:
		using uint64_t = std::uint64_t;
		using int64_t = std::int64_t;
		using uint32_t = std::uint32_t;
		using int32_t = std::int32_t;
	public:
		Analyser(std::vector<Token> v)
			: _tokens(std::move(v)), _offset(0),  _current_pos(0, 0),
              _constants({}),_start({}),  _g_vars({}),_nextTokenIndex(0) {}
		Analyser(Analyser&&) = delete;
		Analyser(const Analyser&) = delete;
		Analyser& operator=(Analyser) = delete;

		// 唯一接口
		std::pair<std::pair<std::vector<Constants>,std::vector<Function>>, std::optional<CompilationError>> Analyse();
	    std::vector<Function> getFunctions() const { return _functions;}
        std::vector<Instruction> getStart()  const { return _start;}

	private:
		// 所有的递归子程序

		// <程序> C0-program
		std::optional<CompilationError> analyseProgram();
        // <变量声明> <variable-declaration>
        std::optional<CompilationError> analyseVariableDeclaration();
        // <function-definition>
        std::optional<CompilationError> analyseFunction();


        // <compound-statement>
        std::optional<CompilationError> analyseCompoundStatement(bool uplevel);
        // <语句序列><statement-seq> ::= {<statement>}
        std::optional<CompilationError> analyseStatementSequence(bool flag);
        // <condition-statement>
        std::optional<CompilationError> analyseConditionStatement();
        // <loop-statement>
        std::optional<CompilationError> analyseLoopStatement();
        // <jump-statement>
        std::optional<CompilationError> analyseJumpStatement();
        // <print-statement>
        std::optional<CompilationError> analysePrintStatement();
        // <scan-statement>
        std::optional<CompilationError> analyseScanStatement();
        // <assignment-expression>
        std::optional<CompilationError> analyseAssignmentExpression();
        // <function-call>
        std::optional<CompilationError> analyseFunctionCall(CONST_TYPE type);

		std::optional<CompilationError> analyseExpression(CONST_TYPE type);

		// <项>
		std::optional<CompilationError> analyseMultiExpression(CONST_TYPE type);
		// <因子>
		std::optional<CompilationError> analyseCastExpression(CONST_TYPE type);


		// Token 缓冲区相关操作

		// 返回下一个 token
		std::optional<Token> nextToken();
		// 回退一个 token
		void unreadToken();

		// 下面是符号表相关操作

		// 是否被声明过
        std::pair<bool, int> isDeclared(const std::string &s);
		// 是否是未初始化的变量
		bool isUninitializedVariable(const std::string&);
		// 是否是已初始化的变量
		bool isInitializedVariable(const std::string&);
		// 是否是常量
        void addLocalConstant(const Token &tk, CONST_TYPE type);

        // 加入Constants常量表
		bool isConstant(const std::string&);
		// 获得 {变量，常量} 在栈上的偏移
		// 与当前层次差，index
        std::pair<int32_t, Var> getVarInfo(const std::string &s);


	private:
		std::vector<Token> _tokens;
		std::size_t _offset;
		std::pair<uint64_t, uint64_t> _current_pos;
        std::vector<Constants> _constants;
        std::vector<Function> _functions;
        std::vector<Instruction> _start;
		// 为了简单处理，我们直接把符号表耦合在语法分析里
		// 变量                   示例
		// _uninitialized_vars    var a;
		// _vars                  var a=1;
		// _consts                const a=1;
		VarList _g_vars;
//		std::map<std::string, int32_t> _g_uninitialized_vars;
//		std::map<std::string, int32_t> _g_vars;
//        std::map<std::string, int32_t>  _g_consts;
//        std::map<std::string, int32_t >  _g_double; // const 在. constants 里的index和名字
//        std::map<std::string, int32_t>  _g_char;
        std::vector<Function>  _current_function;

    public:
        std::vector<CONST_TYPE> _expression_level;

    private:

        //flag
        bool _isStart = true;

        int32_t _nextConstIndex=0;
        int32_t _nextFunIndex=0;


		// 下一个 token 在栈的偏移
		int32_t _nextTokenIndex;

        std::optional<CompilationError> analyseInitDeclaratorList(CONST_TYPE type, bool isConst);

        std::optional<CompilationError> analyseUnaryExpression(CONST_TYPE type);

        void addInstruction( CONST_TYPE type ,Operation op, int32_t x);

        std::optional<CompilationError> analyseIdentifier(CONST_TYPE type);


        bool isFunction(const std::string &s);

        int32_t getFunctionIndex(const std::string &s);

        std::optional<CompilationError> analyseParameterClause();

        Function getFunction(const std::string &s);

        std::optional<CompilationError> analyseCondition();

        void addInstruction(CONST_TYPE type, Operation op, int32_t x, int32_t y);

        void addUninitializedVariable(const Token &tk, CONST_TYPE type);

        void addUninitializedDouble(const Token &tk);


        void addDoubleConst(const Token &tk);

        void addVariable(const Token &tk, CONST_TYPE type);

        void addDouble(const Token &tk);

        bool canReDefine(const std::string &s);

        void assignVar(const std::string &s);
    };
}
