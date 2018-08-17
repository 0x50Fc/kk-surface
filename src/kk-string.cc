
#include "kk-config.h"
#include "kk-string.h"

#include <sstream>

namespace kk {
    
    
    Boolean CStringHasPrefix(CString string,CString prefix) {
        
        if(string == prefix) {
            return true;
        }
        
        if(prefix == NULL) {
            return true;
        }
        
        if(string == NULL) {
            return false;
        }
        
        size_t n1 = strlen(string);
        size_t n2 = strlen(prefix);
        
        return n1 >= n2 && strncmp(string, prefix, n2) == 0;
    }
    
    Boolean CStringHasSuffix(CString string,CString suffix) {
        
        if(string == suffix) {
            return true;
        }
        
        if(suffix == NULL) {
            return true;
        }
        
        if(string == NULL) {
            return false;
        }
        
        size_t n1 = strlen(string);
        size_t n2 = strlen(suffix);
        
        return n1 >= n2 && strncmp(string + n1 - n2, suffix, n2) == 0;
    }
    
    size_t CStringLength(CString string){
        if(string == NULL) {
            return 0;
        }
        return strlen(string);
    }
    
    Boolean CStringEqual(CString string,CString value) {
        
        if(string == value) {
            return true;
        }
        
        if(value == NULL || string == NULL ) {
            return false;
        }
        
        return strcmp(string, value) == 0;
    }

    void CStringSplit(CString string,CString delim, std::vector<String>& items) {
        
        if(string == NULL || delim == NULL) {
            return ;
        }
        
        char * p = (char *) string;
        char * b = p;
        size_t n= strlen(delim);
        
        while(*b != 0) {
            if(strncmp(b, delim, n) == 0) {
                items.push_back(String(p,b-p));
                b += n;
                p = b;
            } else {
                b ++;
            }
        }
        
        if(b != p) {
            items.push_back(String(p,b-p));
        }
        
    }
    
    void CStringSplit(CString string,CString delim, std::set<String>& items) {
        
        if(string == NULL || delim == NULL) {
            return ;
        }
        
        char * p = (char *) string;
        char * b = p;
        size_t n= strlen(delim);
        
        while(*b != 0) {
            if(strncmp(b, delim, n) == 0) {
                items.insert(String(p,b-p));
                b += n;
                p = b;
            } else {
                b ++;
            }
        }
        
        if(b != p) {
            items.insert(String(p,b-p));
        }
        
    }
    
    
    String CStringJoin(std::vector<String>& items,CString delim){
        std::stringstream ss;
        std::vector<String>::iterator i = items.begin();
        
        while(i !=items.end()){
            if(i !=items.begin()){
                ss<< delim;
            }
            ss << * i;
            i ++;
        }
        
        ss << std::ends;
        
        return ss.str();
    }
    
    String CStringJoin(std::set<String>& items,CString delim){
        std::stringstream ss;
        std::set<String>::iterator i = items.begin();
        
        while(i !=items.end()){
            if(i !=items.begin()){
                ss<< delim;
            }
            ss << * i;
            i ++;
        }
        
        ss << std::ends;
        
        return ss.str();
    }
    
    
    String& CStringTrim(String& s){
        
        if (s.empty())
        {
            return s;
        }
        
        s.erase(0,s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
        return s;
    }
    
    String CStringPathAppend(CString basePath,CString path) {
        
        if(basePath == nullptr) {
            return path;
        }
        
        size_t n = strlen(basePath);
        
        if(n == 0) {
            return path;
        }
        
        if(basePath[n - 1] == '/') {
            return String(basePath) + path;
        }
        
        return String(basePath) + "/" + path;
    }
    
    String CStringPathDeleteLast(CString path) {
        
        if(path == nullptr) {
            return "";
        }
        kk::String v = path;
        
        size_t n = v.find_last_of("/");
        
        if(n == kk::String::npos) {
            return v;
        }
        
        return v.substr(0,n + 1);
        
    }
    
    String CStringPathDeleteExtension(CString path) {
        
        if(path == nullptr) {
            return "";
        }
        
        kk::String v = path;
        
        size_t n = v.find_last_of(".");
        
        if(n == kk::String::npos) {
            return v;
        }
        
        return v.substr(0,n);
    }
}
