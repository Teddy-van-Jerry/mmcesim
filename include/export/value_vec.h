#ifndef _EXPORT_VALUE_VEC_H_
#define _EXPORT_VALUE_VEC_H_

#include <vector>
#include <boost/tokenizer.hpp>
#include "yaml.h"
#include "utils.h"

template<typename T>
class Value_Vec {
public:
    Value_Vec(const std::string& str, bool error_out_of_bound = false, T out_of_bound_val = 0);

    Value_Vec(const YAML::Node& node, bool error_out_of_bound = false, T out_of_bound_val = 0);

    T operator[](size_t index) const;

private:
    std::string removeBracket(const std::string& str);

    void parseToken(const std::string& s);

private:
    std::vector<T> _data;
    bool _error_out_of_bound;
    T _out_of_bound_val;
};

template<typename T>
Value_Vec<T>::Value_Vec(const std::string& str, bool error_out_of_bound, T out_of_bound_val)
    : _error_out_of_bound(error_out_of_bound), _out_of_bound_val(out_of_bound_val) {
    std::vector<std::string> tokens;
    std::stringstream ss(removeBracket(str));
    std::string buf;
    // split by ','
    while (ss.good()) {
        std::getline(ss, buf, ',');
        tokens.push_back(buf);
    }
    for (auto&& token : tokens) {
        parseToken(token);
    }
}

template<typename T>
Value_Vec<T>::Value_Vec(const YAML::Node& node, bool error_out_of_bound, T out_of_bound_val) {
    for (auto&& token : node) {
        parseToken(token.as<std::string>());
    }
}

template<typename T>
inline std::string Value_Vec<T>::removeBracket(const std::string& str) {
    if (str.size() < 2) return str;
    if (str[0] == '[' && *(str.end() - 1) == ']') return str.substr(1, str.length() - 2);
    else return str;
}

template<typename T>
void Value_Vec<T>::parseToken(const std::string& s) {
    // find first ':'
    size_t first_colon = s.find(':');
    size_t second_colon = std::string::npos;
    if (first_colon != std::string::npos) {
        second_colon = s.find(':', first_colon + 1);
        if (second_colon != std::string::npos) {
            // 2 colons
            T v1 = strAs<T>(s.substr(0, first_colon));
            T v_ = strAs<T>(s.substr(first_colon + 1, second_colon - first_colon));
            T v2 = strAs<T>(s.substr(second_colon + 1, s.size() - 1 - second_colon));
            if (v_ == 0) {
                if (v1 == v2) {
                    _data.push_back(strAs<T>(s));
                } else {
                    // ERROR! step is stuck at 0.
                    // TODO: error handling
                    assert("step size as 0 in 2 colons specification.");
                }
            } else if (v_ > 0) {
                for (T v = v1; v <= v2 + 1E-12; v += v_) {
                    _data.push_back(v);
                }
            } else {
                for (T v = v1; v + 1E-12 >= v2; v += v_) {
                    _data.push_back(v);
                }
            }
        } else {
            // 1 colon
            T v1 = strAs<T>(s.substr(0, first_colon));
            T v2 = strAs<T>(s.substr(first_colon + 1, s.size() - 1 - first_colon));
            if (v1 <= v2) {
                for (T v = v1; v <= v2 + 1E-12; v += 1) {
                    _data.push_back(v);
                }
            } else {
                // avoid the unsigned problem when subtracting from 0;
                for (T v = v2 + 1; v + 1E-12 >= v1 + 1; v -= 1) {
                    _data.push_back(v - 1);
                }
            }
        }
    } else {
        _data.push_back(strAs<T>(s));
    }
}

template<typename T>
T Value_Vec<T>::operator[](size_t index) const {
    if (index < _data.size()) {
        return _data[index];
    } else {
        if (_error_out_of_bound) {
            // TODO: needs an elegant way of raising error.
            assert("Out of bound for a value_vec!");
        }
        return _out_of_bound_val;
    }
}

#endif