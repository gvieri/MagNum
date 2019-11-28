#ifndef NUMBER_H
#define NUMBER_H

#include <iostream>
#include <stdexcept>
#include <vector> 
#include <algorithm> 
#include <string>

class Number {
    public:
        static int DIVISION_PRECISION;
        static const Number INFINITE, NOT_A_NUMBER;

        Number() = default;
        Number(std::string number) { this->set(number); }
        Number(Number* number) { 
            this->integer = number->integer; this->decimal = number->decimal;
            this->is_decimal = number->is_decimal; this->is_negative = number->is_negative; 
        }

        void operator=(const std::string& number) { this->set(number); }

        void set(std::string num) {
            this->is_negative = false;
            this->integer = "";
            this->is_decimal = false; this->decimal = "";
            
            std::transform(num.begin(), num.end(), num.begin(), [](unsigned char c){ return std::tolower(c); });

            if(num == "nan") { this->integer = "NaN"; return; }
            if(num == "infinite") { this->integer = "INFINITE"; return; }

            std::string number = num;

            if(number.at(0) == '+' || number.at(0) == '-') {
                if(number.at(0) == '-') this->is_negative = true;
                else this->is_negative = false;
                number.erase(0, 1);
            }

            if(number.at(0) == '.') { this->is_decimal = true; number.erase(0, 1); }
            else this->is_decimal = false;

            while(number.length() > 0) {
                char digit = number.at(0);

                if(digit == '.') {
                    if(!this->is_decimal) this->is_decimal = true;
                    else throw std::invalid_argument("Invalid number format: " + num);
                }
                else {
                    if(isdigit(digit)) { this->is_decimal ? this->decimal += digit : this->integer += digit; }
                    else throw std::invalid_argument("Invalid number format: " + num);
                }

                number.erase(0, 1);
            }

            if(!this->integer.empty() && this->integer.at(0) == '0') this->integer.erase(0, this->integer.find_first_not_of('0'));
            if(!this->decimal.empty() && this->decimal.at(this->decimal.length() - 1) == '0') this->decimal.erase(this->decimal.find_last_not_of('0') + 1, this->decimal.length());
        }

        std::string get() { 
            if(this->integer == "NaN") return "NaN";
            if(this->integer == "INFINITE") return "INFINITE";

            std::string number = this->integer.empty() ? "0" : this->integer;

            if(this->is_decimal) { number += '.'; number += this->decimal.empty() ? "0" : this->decimal; }
            if((!this->integer.empty() || !this->decimal.empty()) && this->is_negative) number = "-" + number;

            return number; 
        }

        friend std::ostream & operator << (std::ostream &out, Number& self) { out << self.get(); return out; }

        Number operator+() { return *this; }
        Number operator-() {
           Number result(this);
           result.is_negative = !result.is_negative;
           return result;
        }

        bool operator==(Number& num) { return (this->integer == num.integer) && (this->decimal == num.decimal); }    
        bool operator!=(Number& num) { return (this->integer != num.integer) || (this->decimal != num.decimal); }  

        bool operator<(Number& num) {
            Number self(this);

            if(self == num) return false;

            bool neg = false;

            if(self.is_negative && num.is_negative) neg = true;

            if(self.is_negative ^ num.is_negative) {
                if(self.is_negative) return true;
                if(num.is_negative) return false;
            }

            std::string left_integer = self.integer, left_decimal = self.decimal;
            std::string right_integer = num.integer, right_decimal = num.decimal;
            
            while(left_integer.length() != right_integer.length()) {
                if(left_integer.length() < right_integer.length()) left_integer = "0" + left_integer;
                else right_integer = "0" + right_integer;
            }

            while(left_decimal.length() != right_decimal.length()) {
                if(left_decimal.length() < right_decimal.length()) left_decimal += "0";
                else right_decimal += "0";
            }

            std::string left = left_integer + left_decimal, right = right_integer + right_decimal;

            int l = left.length(), r = right.length(); 
  
            if (l < r) return neg ? false : true; 
            if (r > l) return neg ? true : false; 
  
            for (int i = 0; i < l; i++) { 
                if (left[i] < right[i]) return neg ? false : true; 
                else if (left[i] > right[i]) return neg ? true : false; 
            } 
            return neg ? true : false; 
        }    

        bool operator>(Number& num) { Number self(this); if(self == num) return false; else return !(self < num); }  

        bool operator<=(Number& num) { Number self(this); if(self == num) return true; else return self < num; }  
        bool operator>=(Number& num) { Number self(this); if(self == num) return true; else return !(self < num); }  

        Number operator+(Number& num) {
            Number self(this);

            if(self.integer == "NaN" || num.integer == "NaN") return Number::NOT_A_NUMBER;
            if(self.integer == "INFINITE" || num.integer == "INFINITE") return Number::INFINITE;

            bool neg = false;

            if(self.is_negative && num.is_negative) neg = true;

            if(self.is_negative ^ num.is_negative) {
                if(self.is_negative) {
                    Number n; n = self.get();
                    n.is_negative = false;
                    
                    return num - n;
                }

                if(num.is_negative) {
                    Number n; n = num.get();
                    n.is_negative = false;
                    
                    return self - n;
                }
            }

            size_t dot = self.decimal.length() > num.decimal.length() ? self.decimal.length() : num.decimal.length();

            std::string left_integer = self.integer, left_decimal = self.decimal;
            std::string right_integer = num.integer, right_decimal = num.decimal;
            
            while(left_integer.length() != right_integer.length()) {
                if(left_integer.length() < right_integer.length()) left_integer = "0" + left_integer;
                else right_integer = "0" + right_integer;
            }

            while(left_decimal.length() != right_decimal.length()) {
                if(left_decimal.length() < right_decimal.length()) left_decimal += "0";
                else right_decimal += "0";
            }

            std::string left = left_integer + left_decimal, right = right_integer + right_decimal;

            if(self < num)
                swap(left, right);
 
            int j = left.length()-1;
            for(int i = right.length() - 1; i >= 0; i--, j--)
                left[j] += (right[i] - '0');
 
            for(int i = left.length() - 1; i > 0; i--)
                if(left[i] > '9') {
                    int d = left[i] - '0';
                    left[i-1] = ((left[i-1] - '0') + d / 10) + '0';
                    left[i] = (d % 10) + '0';
                }

            if(left[0] > '9') {
                std::string k;
                k += left[0];
                left[0] = ((left[0] - '0') % 10) + '0';
                k[0] = ((k[0] - '0') / 10) + '0';
                left = k + left;
            }

            left.insert(left.length() - dot, ".");

            Number rslt(left); if(rslt.decimal.empty()) rslt.is_decimal = false;
            if(neg) rslt.is_negative = true;
            return rslt;
        }

        Number operator-(Number& num) {
            Number self(this);

            if(self.integer == "NaN" || num.integer == "NaN") return Number::NOT_A_NUMBER;
            if(self.integer == "INFINITE" || num.integer == "INFINITE") return Number::INFINITE;

            bool neg = false;

            if(self.is_negative && num.is_negative) {
                Number n; n = num.get();
                n.is_negative = false;
                    
                return self + n;
            }

            if(self.is_negative ^ num.is_negative) {
                if(self.is_negative) {
                    Number n; n = num.get();
                    n.is_negative = true;
                    
                    return self + n;
                }

                if(num.is_negative) {
                    Number n; n = num.get();
                    n.is_negative = false;
                    
                    return self + n;
                }
            }

            size_t dot = self.decimal.length() > num.decimal.length() ? self.decimal.length() : num.decimal.length();

            std::string left_integer = self.integer, left_decimal = self.decimal;
            std::string right_integer = num.integer, right_decimal = num.decimal;
            
            while(left_integer.length() != right_integer.length()) {
                if(left_integer.length() < right_integer.length()) left_integer = "0" + left_integer;
                else right_integer = "0" + right_integer;
            }

            while(left_decimal.length() != right_decimal.length()) {
                if(left_decimal.length() < right_decimal.length()) left_decimal += "0";
                else right_decimal += "0";
            }

            std::string left = left_integer + left_decimal, right = right_integer + right_decimal;

            if(self < num) { swap(left, right); neg = true; }
  
            std::string result; 
  
            int n1 = left.length(), n2 = right.length(); 
            int diff = n1 - n2; 
  
            int carry = 0; 
  
            for (int i = n2 - 1; i >= 0; i--) { 
                int sub = ((left[i + diff] - '0') - (right[i] - '0') - carry); 
                if (sub < 0) { 
                    sub = sub + 10; 
                    carry = 1; 
                } 
               else carry = 0; 
  
               result.push_back(sub + '0'); 
            } 
  
            for (int i = n1 - n2 - 1; i >= 0; i--) { 
                if (left[i] == '0' && carry) { 
                    result.push_back('9'); 
                    continue; 
                } 
                int sub = ((left[i] - '0') - carry); 
                if (i > 0 || sub > 0) result.push_back(sub + '0'); 
                carry = 0; 
            } 
  
            std::reverse(result.begin(), result.end()); 
            result.insert(result.length() - dot, ".");

            Number rslt(result); if(rslt.decimal.empty()) rslt.is_decimal = false;
            if(neg) rslt.is_negative = true;
            return rslt;
        }

        Number operator*(Number& num) {
            Number self(this);
        
            if(self.integer == "NaN" || num.integer == "NaN") return Number::NOT_A_NUMBER;
            if(self.integer == "INFINITE" || num.integer == "INFINITE") return Number::INFINITE;

            bool neg = false;
            if(self.is_negative ^ num.is_negative) neg = true;

            size_t dot = self.decimal.length() + num.decimal.length();

            std::string left = self.integer + self.decimal, right = num.integer + num.decimal;

            int len1 = left.length(); 
            int len2 = right.length(); 
            
            if (len1 == 0 || len2 == 0) return Number(); 
  
            std::vector<int> result(len1 + len2, 0); 
   
            int i_n1 = 0;  
            int i_n2 = 0;  
      
            for (int i = len1 - 1; i >= 0; i--) { 
                int carry = 0; 
                int n1 = left[i] - '0'; 
   
                i_n2 = 0;  
                      
                for (int j = len2 - 1; j >= 0; j--) { 
                    int n2 = right[j] - '0'; 
   
                    int sum = n1 * n2 + result[i_n1 + i_n2] + carry; 
  
                    carry = sum / 10; 
  
                    result[i_n1 + i_n2] = sum % 10; 
  
                    i_n2++; 
                } 
   
                if (carry > 0) result[i_n1 + i_n2] += carry; 
 
                i_n1++; 
            } 

            int i = result.size() - 1; 
            while (i >= 0 && result[i] == 0) 
                i--; 
  
            if (i == -1) return Number(); 
  
            std::string s; 
      
            while (i >= 0) 
                s += std::to_string(result[i--]); 
  
            s.insert(s.length() - dot, ".");

            Number rslt(s); if(rslt.decimal.empty()) rslt.is_decimal = false;
            if(neg) rslt.is_negative = true;
            return rslt; 
        }

        Number operator/(Number& num) {
            Number self(this); Number zero;
            
            if(self.integer == "NaN" || num.integer == "NaN") return Number::NOT_A_NUMBER;
            if(self.integer == "INFINITE" || num.integer == "INFINITE") return Number::INFINITE;

            if(self == zero ^ num == zero) {
                if(self == zero) return zero;
                if(num == zero) return Number::NOT_A_NUMBER;
            }

            if(self == zero && num == zero) return Number::INFINITE;

            bool neg = false;
            if(self.is_negative ^ num.is_negative) neg = true;

            std::string left_decimal = self.decimal, right_decimal = num.decimal;

            while(left_decimal.length() != right_decimal.length()) {
                if(left_decimal.length() < right_decimal.length()) left_decimal += "0";
                else right_decimal += "0";
            }

            Number left(self.integer + left_decimal), right(num.integer + right_decimal);
            Number result; 

            int range = right.get().length();
            int precision = 0;

            bool to_add = false;

            while(left != zero && precision < Number::DIVISION_PRECISION) {
                Number ranged(left.get().substr(0, range));
                bool range_plus = false;
                
                if(ranged.get().length() < range) {
                    if(!result.is_decimal) { result.is_decimal = true; to_add = true; }
                    else if (to_add) { result.decimal = result.decimal + zero.get(); precision++; } else to_add = true;
                    left = left.get() + zero.get(); continue;
                }

                if(ranged <= right) {
                    if(left.get().length() - ranged.get().length() >= 1) { ranged = Number(left.get().substr(0, range + 1));  range_plus = true; }
                    else {
                        if(!result.is_decimal) result.is_decimal = true;
                        else if (to_add) { result.decimal = result.decimal + zero.get(); precision++; } else to_add = true;
                        left = left.get() + zero.get(); ranged = Number(left.get().substr(0, range + 1));
                    }
                }

                int digit = 0;

                while(ranged - right >= zero) {
                    to_add = false;
                    ranged = ranged - right;
                    digit++; 
                }

                result.is_decimal ? result.decimal += std::to_string(digit) : result.integer += std::to_string(digit);
                left = left.get().replace(0, range_plus ? range + 1 : range, ranged.get());
                if(result.is_decimal) precision++;
            }

            if(neg) result.is_negative = true;
            return result; 
        }

        Number operator%(Number& num) {
            Number self(this);
        
            if(self.integer == "NaN" || num.integer == "NaN") return Number::NOT_A_NUMBER;
            if(self.integer == "INFINITE" || num.integer == "INFINITE") return Number::INFINITE;

            bool neg = num.is_negative;

            Number zero;
            Number left(self), right(num); left.is_negative = false; right.is_negative = false;

            while(left - right >= zero) left = left - right;

            Number rslt(left);
            if(neg) rslt.is_negative = true;
            return rslt;
        }

    private:
        std::string integer; std::string decimal;
        bool is_negative = false; bool is_decimal = false; 
};

int Number::DIVISION_PRECISION = 20;
const Number Number::INFINITE = Number("INFINITE");
const Number Number::NOT_A_NUMBER = Number("NaN");

#endif
