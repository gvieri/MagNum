#ifndef VOID_H
#define VOID_H

#include <iostream>

class Void {
    public:
        static Void VOID() { return Void(); }

        std::string get() { return Void::name; }
        friend std::ostream& operator<<(std::ostream &out, Void &self) { return out << Void::name; }

        bool operator==(Void& value) { return true; } bool operator!=(Void& value) { return false; } 
    private:
        static const std::string name;
};

const std::string Void::name = "void";

#endif