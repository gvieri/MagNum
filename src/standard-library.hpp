#ifndef STANDARD_LIBRARY_H
#define STANDARD_LIBRARY_H

#include "common.hpp"

Value number(Value* arguments, int parameters) {
    if(parameters != 1) return Void::VOID();
    if(arguments[parameters - 1].index() != VAL_STRING) return Void::VOID();

    String string = arguments[parameters - 1].value.string;
    return Number(string.get());
}

Value string(Value* arguments, int parameters) {
    if(parameters != 1) return Void::VOID();
    if(arguments[parameters - 1].index() != VAL_NUMBER) return Void::VOID();

    Number number = arguments[parameters - 1].value.number;
    return String(number.get());
}

Value length(Value* arguments, int parameters) {
    if(parameters != 1) return Void::VOID();
    if(arguments[parameters - 1].index() != VAL_STRING) return Void::VOID();

    String string = arguments[parameters - 1].value.string;
    return Number(std::to_string(string.get().size()));
}

#endif