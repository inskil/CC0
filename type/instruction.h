#pragma once

#include <cstdint>
#include <utility>

namespace cc0 {

    enum Operation {
        NOP = 0,
        LOADA,
        LOADC,
        IPUSH,
        BIPUSH,
        TLOAD,
        ILOAD,
        DLOAD,
        TSTORE,
        ISTORE,
        DSTORE,
        CSTORE,
        TALOAD,
        TASTORE,
        NEW,
        SNEW,
        POP,
        DUP,
        TADD,
        IADD,
        DADD,
        TSUB,
        ISUB,
        DSUB,
        TMUL,
        IMUL,
        DMUL,
        TDIV,
        IDIV,
        DDIV,
        TNEG,
        INEG,
        DNEG,
        TCMP,
        ICMP,
        DCMP,
        I2D,
        I2C,
        D2I,
        JMP,
        JNE,
        JE,
        JL,
        JGE,
        JG,
        JLE,
        CALL,
        RET,
        TRET,
        IRET,
        DRET,
        TPRINT,
        IPRINT,
        DPRINT,
        CPRINT,
        SPRINT,
        PRINTL,
        TSCAN,
        ISCAN,
        CSCAN,
        DSCAN,

    };

    class Instruction final {
    private:
        using int32_t = std::int32_t;
    public:
        friend void swap(Instruction &lhs, Instruction &rhs);

    public:
        Instruction(Operation opr, int32_t x, int32_t y) : _opr(opr), _x(x), _y(y) {}

        Instruction(Operation opr) : _opr(opr) {};

        Instruction(Operation opr, int32_t x) : _opr(opr), _x(x) {};

        Instruction() : Instruction(Operation::NOP, 0) {}

        Instruction(const Instruction &i) {
            _opr = i._opr;
            _x = i._x;
            _y = i._y;
        }

        Instruction(Instruction &&i) : Instruction() { swap(*this, i); }

        Instruction &operator=(Instruction i) {
            swap(*this, i);
            return *this;
        }

        bool operator==(const Instruction &i) const { return _opr == i._opr && _x == i._x && _y == i._y; }

        Operation GetOperation() const { return _opr; }

        int32_t GetX() const { return _x; }

    private:
        Operation _opr;
    public:
        int32_t getY() const {
            return _y;
        }

        void setOpr(Operation opr) {
            _opr = opr;
        }

        void setX(int32_t x) {
            _x = x;
        }

        void setY(int32_t y) {
            _y = y;
        }

    private:
        int32_t _x;
        int32_t _y;

    };

    inline void swap(Instruction &lhs, Instruction &rhs) {
        using std::swap;
        swap(lhs._opr, rhs._opr);
        swap(lhs._x, rhs._x);
    }

}