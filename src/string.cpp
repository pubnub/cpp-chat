#include "string.hpp"
#include <string>

using namespace Pubnub;

// each +1 is for the null terminator

String::~String() {
    if (this->string != nullptr) {
        delete[] this->string;
        this->string = nullptr;
        this->len = 0;
        this->cap = 0;
    }
}

String::String(const char* string, std::size_t lenght) {
    if(string == nullptr) {
        this->string = nullptr;
        this->len = 0;
        this->cap = 0;

        return;
    }

    init(string, lenght);
}

String::String(const char* string) {
    if (string == nullptr) {
        this->string = nullptr;
        this->len = 0;
        this->cap = 0;

        return;
    }

    init(string, strlen(string));
}

String::String(char* string):
    String(static_cast<const char*> (string)) {}

String::String(std::string string):
    String(string.c_str()) {} 

String::String(const String& string):
    String(string.string) {}

String::String(String&& string) {
    this->string = string.string;
    this->len = string.len;
    this->cap = string.cap;

    string.string = nullptr;
    string.len = 0;
    string.cap = 0;
}

String::operator std::string() const {
    return std::string(this->string);
}

String::operator const char*() const {
    return this->string;
}

String& String::operator=(const char* string) {
    if(string == nullptr) {
        if (this->string != nullptr) {
            delete[] this->string;
        }
        this->string = nullptr;
        this->len = 0;
        this->cap = 0;
        
        return *this;
    }
    
    auto lenght = strlen(string);
    if (this->string != nullptr) {
        delete[] this->string;
    }

    this->string = new char[lenght + 1];
    memcpy(this->string, string, lenght);
    this->string[lenght] = '\0';

    this->len = lenght;
    this->cap = lenght;

    return *this;
}

String& String::operator=(char* string) {
    return this->operator=(static_cast<const char*> (string));
}

String& String::operator=(std::string string) {
    return this->operator=(string.c_str());
}

String& String::operator=(const String& string) {
    return this->operator=(string.string);
}

String& String::operator=(String&& string) {
    return this->operator=(string.string);
}

String& String::operator+=(const char* string) {
    if (string == nullptr) {
        return *this;
    }
    auto lenght = strlen(string);

    auto new_len = this->len + lenght;
    this->grow_if_needed(new_len);

    strcat(this->string, string);
    this->len = new_len;

    return *this;
}

String& String::operator+=(const String& string) {
    return this->operator+=(string.string);
}

String& String::operator+=(String& string) {
    return this->operator+=(string.string);
}

String& String::operator+=(std::string string) {
    return this->operator+=(string.c_str());
}

const char* String::c_str() const {
    return this->string;
}

char* String::c_str() {
    return this->string;
}

std::string String::to_std_string() const {
    return std::string(this->string);
}

std::size_t String::length() const {
    return this->len;
}

std::size_t String::capacity() const {
    return this->cap;
}

bool String::empty() const {
    if (this->string == nullptr) {
        return true;
    }

    return this->len == 0;
}

void String::clear() {
    if (this->string != nullptr) {
        memset(this->string, 0, this->cap);
        this->len = 0;
    }
}

void String::erase(std::size_t pos, std::size_t count) {
    if (pos >= this->len) {
        return;
    }

    if (pos + count >= this->len) {
        this->string[pos] = '\0';
        this->len = pos;

        return;
    }

    auto new_len = this->len - count;
    for (auto i = pos; i < new_len; i++) {
        this->string[i] = this->string[i + count];
    }
    this->string[new_len] = '\0';

    this->len = new_len;
}

void String::insert(std::size_t pos, char character) {
    if (pos >= this->len) {
        return;
    }

    auto new_len = this->len + 1;
    this->grow_if_needed(new_len);

    for (auto i = this->len; i > pos; i--) {
        this->string[i] = this->string[i - 1];
    }
    this->string[pos] = character;
    this->string[new_len] = '\0';

    this->len = new_len;
}

void String::shrink() {
    auto new_cap = this->len;
    auto new_string = new char[new_cap + 1];
    memcpy(new_string, this->string, new_cap);

    delete[] this->string;
    this->string = new_string;
    this->cap = new_cap;
}

std::size_t String::find(const char* string, std::size_t pos) const {
    auto string_len = strlen(string);
    if (string_len == 0) {
        return pos;
    }

    if (pos >= this->len) {
        return String::npos;
    }

    for (auto i = pos; i < this->len; i++) {
        if (this->string[i] == string[0]) {
            auto found = true;
            for (auto j = 1; j < string_len; j++) {
                if (this->string[i + j] != string[j]) {
                    found = false;
                    break;
                }
            }

            if (found) {
                return i;
            }
        }
    }

    return String::npos;
}

void String::replace(std::size_t pos, std::size_t count, const char* string) {
    if (pos > this->len) {
        return;
    }

    auto string_len = strlen(string);
    auto new_len = this->len - count + string_len;
    this->grow_if_needed(new_len);

    if (string_len > count) {
        for(auto i = new_len; i >= pos + string_len; i--) {
            this->string[i] = this->string[i - string_len + count];
        }

        for (auto i = 0; i < string_len; i++) {
            this->string[pos + i] = string[i];
        }
    } else {
        for (auto i = 0; i < string_len; i++) {
            this->string[pos + i] = string[i];
        }

        for (auto i = pos + string_len; i < this->len; i++) {
            this->string[i - string_len + count] = this->string[i];
        }
    }

    this->string[new_len] = '\0';
    this->len = new_len;
}

void String::replace(std::size_t pos, std::size_t count, const String& string) {
    this->replace(pos, count, string.string);
}

void String::replace_all(const char* string, const char* replacement) {
    auto string_len = strlen(string);
    auto replacement_len = strlen(replacement);

    auto pos = this->find(string);
    while (pos != String::npos) {
        this->replace(pos, string_len, replacement);
        pos = this->find(string, pos + replacement_len);
    }
}

void String::replace_all(const std::string& string, const std::string& replacement) {
    this->replace_all(string.c_str(), replacement.c_str());
}

void String::replace_all(const String& string, const String& replacement) {
    this->replace_all(string.string, replacement.string);
}

String String::substring(std::size_t pos, std::size_t count) const {
    if (pos >= this->len) {
        return String();
    }

    auto new_len = count;
    if (pos + count > this->len) {
        new_len = this->len - pos;
    }

    String new_string;
    new_string.reserve(new_len);

    for (auto i = pos; i < new_len + pos; i++) {
        new_string.begin()[i - pos] = this->string[i];
    }

    new_string.len = new_len;

    return new_string;
}

void String::grow_if_needed(std::size_t new_len) {
    if (new_len > this->cap) {
        auto new_cap = this->calculate_capacity(new_len);

        auto new_string = new char[new_cap + 1];
        memset(new_string, 0, new_cap + 1);
        if (this->string != nullptr) {
            memcpy(new_string, this->string, this->len + 1);
        }

        delete[] this->string;
        this->string = new_string;
        this->cap = new_cap;
    }
}

const char* String::begin() const {
    return this->string;
}

const char* String::end() const {
    return this->string + this->len;
}

char* String::begin() {
    return this->string;
}

char* String::end() {
    return this->string + this->len;
}

const char& String::front() const {
    return this->string[0];
}

const char& String::back() const {
    return this->string[this->len - 1];
}

char& String::front() {
    return this->string[0];
}

char& String::back() {
    return this->string[this->len - 1];
}

char* String::into_c_str() {
    auto result = this->string;
    this->string = nullptr;
    this->len = 0;
    this->cap = 0;

    return result;
}

void String::reserve(std::size_t new_cap) {
    if (new_cap > this->cap) {
        auto new_string = new char[new_cap + 1];
        memset(new_string, 0, new_cap + 1);

        if (string != nullptr) {
            memcpy(new_string, this->string, new_cap);
        } 

        delete[] this->string;
        this->string = new_string;
        this->cap = new_cap;
    } else {
        this->shrink();
    }
}

void String::init(const char* string, std::size_t lenght) {
    this->string = new char[lenght + 1];
    memcpy(this->string, string, lenght);
    this->string[lenght] = '\0';

    this->len = lenght;
    this->cap = lenght;
}

std::size_t String::calculate_capacity(std::size_t len) const {
    // For calculating the capacity we used the Rust's formula
    // for the Vec's capacity calculation
    // https://doc.rust-lang.org/1.61.0/src/alloc/raw_vec.rs.html#394
    auto doubled_capacity = this->cap * 2;

    return doubled_capacity > len ? doubled_capacity : len;
}

String operator+(const String& lhs, const String& rhs) {
    String new_string;
    new_string.reserve(lhs.length() + rhs.length());
    new_string += lhs;
    new_string += rhs;

    return new_string;
}

bool operator==(const String& lhs, const String& rhs) {
    if(lhs.c_str() == nullptr || rhs.c_str() == nullptr) {
        return false;
    }

    return strcmp(lhs.c_str(), rhs.c_str()) == 0;
}

bool operator!=(const String& lhs, const String& rhs) {
    return strcmp(lhs.c_str(), rhs.c_str()) != 0;
}

