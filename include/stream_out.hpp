#ifndef STREAM_OUT_HPP
#define STREAM_OUT_HPP

#include <vector>
#include <ostream>

namespace std {

template<typename T>
ostream& operator<<(ostream& os, const vector<T>& v) {
    os << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        os << +v[i];
        if (i + 1 < v.size()) os << ", ";
    }
    os << "]";
    return os;
}

}

#endif
