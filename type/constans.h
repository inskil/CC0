#pragma once

#include <cstdint>
#include <utility>
#include <string>

namespace cc0 {

    enum CONST_TYPE{
        I = 2,
        S = 4,
        D  =3,
        C = 1,
        V = 5,
        N = 0 //NULL
    };

    class Constants final {

    private:
        using int32_t = std::int32_t;
    public:
        friend void swap(Constants& lhs, Constants& rhs);
    public:
        Constants(CONST_TYPE constType,const std::string  &str,int32_t index) : _constType(constType), _str(str), _index(index)  {}

        CONST_TYPE GetConstType() const { return _constType; }

        std::string Get() const { return _str; }

        int32_t GetIndex() const { return _index; }

    private:
        CONST_TYPE _constType;
        std::string _str;
        int32_t _index;
    };

    inline void swap(Constants& lhs, Constants& rhs) {
        using std::swap;
        swap(lhs._constType, rhs._constType);
        swap(lhs._str, rhs._str);
        swap(lhs._index, rhs._index);
    }

}