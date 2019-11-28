#ifndef STRING_H 
#define STRING_H

#include <iostream>
#include <cstring>

class String {
    public:
        String() = default;
        String(std::string content) { this->set(content); }
        String(String* copy) { this->content = copy->content; }

        void operator=(const std::string& str) { this->set(str); }

        friend std::ostream & operator << (std::ostream &out, String &self) { out << '"' << self.get() << '"'; return out; }

        std::string get() { return std::string(this->content); }

        void set(std::string content) {
            this->content = new char[content.length() + 1];
            strcpy(this->content, content.c_str());
        }

        bool operator==(String& str) { return this->get() == str.get(); }    
        bool operator!=(String& str) { return this->get() != str.get(); }  

        bool operator<(String& str) {
            std::string self = std::string(this->content);
            std::string comp = std::string(str.content);

            if(self.length() < comp.length()) return true;
            if(self.length() > comp.length()) return false;

            return self.compare(comp) < 0;
        }   

        bool operator>(String& str) { String self(this); if(self == str) return false; else return !(self < str); }  

        bool operator<=(String& str) { String self(this); if(self == str) return true; else return self < str; }  
        bool operator>=(String& str) { String self(this); if(self == str) return true; else return !(self < str); }

        String operator-() {
           String result(this);

           std::string reversed = std::string(result.content);
           std::reverse(reversed.begin(), reversed.end());
           result = reversed;

           return result;
        }

        String operator+(String& str) {
           std::string result = std::string(this->content) + std::string(str.content);
           return String(result);
        }

        static String CONCATENATE(String left, String right) {
            std::string result = std::string(left.content) + ' ' + std::string(right.content);
            return String(result);
        }

    private:
        char* content;
};

#endif