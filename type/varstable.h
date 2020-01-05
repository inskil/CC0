#pragma once

#include <cstdint>
#include <utility>
#include "./constans.h"
#include <map>

namespace cc0 {
    class Var final {

    private:
        using int32_t = std::int32_t;
    public:
        Var(CONST_TYPE type, int32_t index, bool isUnInit, bool isConst) : _type(type), _index(index),
                                                                           _isUnInit(isUnInit), _isConst(isConst) {}

        CONST_TYPE getType() const {
            return _type;
        }

        void setIsUnInit(bool isUnInit) {
            _isUnInit = isUnInit;
        }

        int32_t getIndex() const {
            return _index;
        }

        bool isIsUnInit() const {
            return _isUnInit;
        }

        bool isIsConst() const {
            return _isConst;
        }

    private:
        CONST_TYPE _type = V;
        int32_t _index = 0;
        bool _isUnInit = true;
        bool _isConst = false;
    };

    class VarList final {

    private:
        using int32_t = std::int32_t;

    public:
        // name var , index = var index
        std::map<std::string, Var> _varsList;
        int32_t _level;

        VarList(int32_t level) : _level(level), _varsList(){}

        Var &getVar(const std::string &s) {
            if (_varsList.find(s) != _varsList.end())
                for (auto &it :_varsList) {
                    if (it.first == s)return it.second;
                }
        }

        bool isDeclared(const std::string &s) {
            return _varsList.find(s) != _varsList.end();
        }

        void addVar(const std::string &name, CONST_TYPE type, int32_t index, bool isUnInit, bool isConst) {
            _varsList.insert(std::map<std::string,Var>::value_type(name,Var(type, index, isUnInit, isConst)));
        }

        void assignVar(const std::string &name) {
            if (_varsList.find(name) != _varsList.end())
                for (auto &it :_varsList)
                    if (it.first == name) {
                        it.second.setIsUnInit(false);
                    }
        }
    };

    class VarsTable final {
    private:
        using int32_t = std::int32_t;
    public:
        friend void swap(Instruction &lhs, Instruction &rhs);

        VarsTable(){
            _vars.emplace_back(1);
            _maxlevel = 1;
        }

    public:
        std::vector<VarList> _vars;
        int32_t _maxlevel = 0;

    public:
        // if type = D , INDEX = Const index
        Var &getVar(const std::string &s) {
            for (auto &list :_vars) {
                if (list._varsList.find(s) != list._varsList.end())return list.getVar(s);
            }
        }

        bool isDeclared(const std::string &s) {
            for (auto &list :_vars) {
                if (list._varsList.find(s) != list._varsList.end())return true;
            }
            return false;
        }


        bool isConst(const std::string &s) {
            auto var = getVar(s);
            return var.isIsConst();
        }

        void addVar(const std::string &name, CONST_TYPE type, int32_t index, bool isUnInit, bool isConst) {
            _vars.back().addVar(name, type, index, isUnInit, isConst);
        }

        // 退出最后一个层次
        void quitLevel() {
            _maxlevel--;
            _vars.erase(_vars.end());
        }

        void addLevel() {
            _maxlevel++;
            _vars.emplace_back(_maxlevel);
        }

        bool canReDefine(const std::string &s) {
            for (auto &list :_vars) {
                if (list._varsList.find(s) != list._varsList.end()) {
                    if (list._level == _maxlevel) return false;
                }
            }
            return true;
        }

        void assignVar(const std::string &s) {
            for (auto &list :_vars) {
                if (list._varsList.find(s) != list._varsList.end()) {
                   list.assignVar(s);
                }
            }
        }

    };
}