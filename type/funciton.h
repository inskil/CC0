#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include <map>
#include "tokenizer/token.h"
#include "./varstable.h"

namespace cc0 {

    class Function final {
    private:
        using int32_t = std::int32_t;

    private:
        //name string in const table
        int32_t _name_index;
        int32_t _params_size = 0;
        // funciton index
        int32_t _index;
        int32_t _current_level=1;
        std::string _name;
        // all var index
        int32_t _nextTokenIndex = 0;
        bool isfunction = true;
        std::map<int32_t , CONST_TYPE> _params;
        CONST_TYPE _re_type;

    private:
        // all type var in here
        VarsTable _allVar;// 如果为double 则为 double 在. constants 里的index和名字




    public:
        friend void swap(Function &lhs, Function &rhs);

        int32_t getParamNum() const {
            return _params.size();
        }

        Function(int32_t nameIndex, int32_t paramsSize, int32_t index, int32_t level) : _name_index(nameIndex),
                                                                                        _params_size(paramsSize),
                                                                                        _index(index),
                                                                                        _current_level(level){}

        Function(int32_t nameIndex, int32_t index, int32_t level) : _name_index(nameIndex), _index(index),
                                                                    _current_level(level) { _params_size = 0; }


        int32_t getNameIndex() const {
            return _name_index;
        }

        int32_t getParamsSize() const {
            return _params_size;
        }

        int32_t getIndex() const {
            return _index;
        }

        int32_t getLevel() const {
            return _current_level;
        }

        CONST_TYPE getReType() const {
            return _re_type;
        }


        const std::vector<Instruction> &getInstructions() const {
            return _instructions;
        }

        void changeJumpX(int32_t index, int32_t x) {
            _instructions.at(index).setX(x);
        }


        bool paramsAdd(CONST_TYPE type) {
            auto size = 0;
            if (type == D)size = 2;
            else size = 1;
            _params_size += size;
            _params[getParamNum()] = type;
            return true;
        };

        void addInstruction(Operation op, int32_t x, int32_t y) {
            _instructions.emplace_back(op, x, y);
        }

        void addInstruction(Operation op, int32_t x) {
            _instructions.emplace_back(op, x);
        }


    public:
        const std::string &getName() const {
            return _name;
        }

        Function(int32_t nameIndex, std::string &name, int32_t index, int32_t level, bool isFunction,
                CONST_TYPE re_type) : _name_index(nameIndex), _index(index), _current_level(level), _name(name),
                                       isfunction(isFunction), _params({}),
                                       _re_type(re_type) {}


    public:
        Function() {}

        const std::map<int32_t , CONST_TYPE> &getParams() const {
            return _params;
        }

        // 添加int char常量
        void addLocalConstant(const Token &tk, CONST_TYPE type) {
            _allVar.addVar(tk.GetValueString(), type, _nextTokenIndex++, false, true);
        }


        // 添加int char变量
        void addVariable(const Token &tk, CONST_TYPE type) {
            _allVar.addVar(tk.GetValueString(), type, _nextTokenIndex++, false, false);
        }

        // 添加int char未初始化变量
        void addUninitializedVariable(const Token &tk, CONST_TYPE type) {
            _allVar.addVar(tk.GetValueString(), type, _nextTokenIndex++, true, false);
        }

        // 添加double未初始化变量
        void addUninitializedDouble(const Token &tk,int32_t index) {
            _allVar.addVar(tk.GetValueString(), D, index, true, false);
        }

        // 添加 double 已经初始化变量
        void addDouble(const Token &tk,int32_t index) {
            _allVar.addVar(tk.GetValueString(), D, index, false, false);
        }

        // 添加 double 常量
        void addDoubleConst(const Token &tk,int32_t index) {
            _allVar.addVar(tk.GetValueString(), D, index, false, true);
        }

        int32_t getInsLen() { return _instructions.size(); };

        // 是否被声明过
        bool isDeclared(const std::string &s) {
            return _allVar.isDeclared(s);
        }

        // 是否是未初始化的变量
        bool isUninitializedVariable(const std::string &s) {
            return _allVar.getVar(s).isIsUnInit();
        }

        // 是否是已初始化的变量
        bool isInitializedVariable(const std::string &s) {
            return !isUninitializedVariable(s);
        }

        // 是否是常量
        bool isConstant(const std::string &s) {
            return _allVar.getVar(s).isIsConst();
        }

        // 获得 int char  在栈上的偏移  or double 在的常量index
        Var getVarInfo(const std::string &s) {
            return _allVar.getVar(s);
        }

        bool canReDefine(const std::string &s){
            return _allVar.canReDefine(s);
        }

        // 新增一个作用域
        void addLevel() {
            return _allVar.addLevel();
        }

        // 新增一个作用域
        void downLevel() {
            return _allVar.quitLevel();
        }

        void assignVar(const std::string &s) {
            _allVar.assignVar(s);
        }


    private:
        std::vector<Instruction> _instructions;

    };

    inline void swap(Function &lhs, Function &rhs) {
        using std::swap;
        swap(lhs._index, rhs._index);
        swap(lhs._name_index, rhs._name_index);
        swap(lhs._params_size, rhs._params_size);
        swap(lhs._current_level, rhs._current_level);
    }



}