#ifndef BOOLEAN_H 
#define BOOLEAN_H

#include <iostream>

class Boolean {
    public:
        bool boolean;

        Boolean() = default;
        Boolean(bool boolean) { this->set(boolean); }
        Boolean(Boolean* copy) { this->boolean = copy->boolean; }

        static Boolean TRUE() { return Boolean(true); } 
        static Boolean FALSE() { return Boolean(false); }

        void operator=(const bool& boolean) { this->set(boolean); }

        friend std::ostream & operator << (std::ostream &out, Boolean &self) { out << self.get(); return out; }

        std::string get() { return this->boolean ? "true" : "false"; }

        void set(bool boolean) { this->boolean = boolean; }

        bool operator==(Boolean& boolean) { return this->boolean == boolean.boolean; }    
        bool operator!=(Boolean& boolean) { return this->boolean != boolean.boolean; } 

        Boolean operator&&(Boolean& comp) {
            Boolean self(this); if(!self.boolean) return Boolean(false);
        
            if(self.boolean && comp.boolean) return Boolean(true);
            return Boolean(false);
        }

        Boolean operator||(Boolean& comp) {
            Boolean self(this); if(self.boolean) return Boolean(true);
        
            if(self.boolean || comp.boolean) return Boolean(true);
            return Boolean(false);
        }

        Boolean operator!() { Boolean self(this); return self.boolean ? Boolean(false) : Boolean(true); }
};

#endif