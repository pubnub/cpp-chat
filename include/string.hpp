#ifndef PN_STRING_H
#define PN_STRING_H

#include "export.hpp"
#include <cstddef>
#include <string>

namespace Pubnub {
    /**
     * The String class to handle strings in the Pubnub SDK.
     *
     * It wraps the const char* string and provides methods to manipulate it 
     * without the need to worry about memory management and dll boundaries.
     */
    class PN_CHAT_EXPORT String 
    {
        //TODO: check FString from Unreal Engine
    public:
        /**
         * Default constructor that leaves the string empty.
         */
        String() = default;
        ~String();

        /**
         * Constructor that initializes the string with a const char* string.
         * 
         * @param string The const char* string to initialize the String with.
         */
        String(const char* string);

        /**
         * Constructor that initializes the string with a std::string.
         * 
         * @param string The char* to initialize the String with.
         */
        String(char* string);

        /**
         * Constructor that initializes the string with a std::string.
         * 
         * @param string The std::string to initialize the String with.
         */
        String(std::string string);

        /**
         * Copy constructor that initializes the string with another String.
         * 
         * @param string The std::string const reference to initialize the String with.
         */
        String(const String& string);

        /**
         * Move constructor that initializes the string with another String.
         * 
         * @param string The std::string rvalue reference to initialize the String with.
         */
        String(String&& string);

        /**
         * Implicit conversion operator that converts the String to a std::string.
         * 
         * @return The std::string string.
         */
        operator std::string() const;

        /**
         * Implicit conversion operator that converts the String to a const char*.
         * 
         * @return The const char* string.
         */
        operator const char*() const;

        /**
         * Copy assignment operator that assigns the string with another String.
         * 
         * @param string The std::string const reference to assign the String with.
         */
        String& operator=(const String& string) = default;

        /**
         * Move assignment operator that assigns the string with another String.
         * 
         * @param string The std::string rvalue reference to assign the String with.
         */
        String& operator=(String&& string) = default;

        /**
         * Copy assignment operator that assigns the string with a const char* string.
         * 
         * @param string The const char* to assign the String with.
         */
        String& operator=(const char* string);

        /**
         * Add assignment operator that appends the string with another String.
         *
         * @param string The std::string const reference to append the String with.
         */
        String& operator+=(const String& string);

        /**
         * Add assignment operator that appends the string with a const char* string.
         *
         * @param string The const char* to append the String with.
         */
        String& operator+=(const char* string);

        /**
         * Add assignment operator that appends the string with a std::string.
         *
         * @param string The std::string to append the String with.
         */
        String& operator+=(std::string string);

        /**
         * Copy assignment operator that assigns the string with a std::string.
         * 
         * @param string The std::string to assign the String with.
         */
        String& operator=(std::string string);

        /**
         * Returns raw const char* pointer to the string that the String holds.
         *
         * @return The const char* string.
         */
       const char* c_str() const;

        /**
         * Returns std version of the string that the String holds.
         *
         * @return The std::string string.
         */
       std::string to_std_string() const;

        /**
         * Returns the length of the string that the String holds.
         *
         * @return The length of the string.
         */
       std::size_t length() const;

        /**
         * Returns the capacity of the string that the String holds.
         * 
         * Capacity is the size of the allocated storage for the string, a value that is at least as large as length.
         *
         * @return The capacity of the string.
         */
       std::size_t capacity() const;

       /**
        * Checks if the string is empty.
        *
        * @return True if the string is empty, false otherwise.
        */
       bool empty() const;

       /**
        * Clears the string.
        */
       void clear();

       /**
        * Erases the character(s) at the specified position.
        *
        * @param pos The position of the character to erase.
        * @param count The number of characters to erase.
        */
       void erase(std::size_t pos, std::size_t count = 1);

       /**
        * Inserts the character at the specified position.
        *
        * @param pos The position to insert the character at.
        * @param character The character to insert.
        */
       void insert(std::size_t pos, char character);

       /**
        * Finds the first occurrence of the character or sequence of characters in the string.
        *
        * @param string The character or sequence of characters to find.
        * @param std::size_t The position to start the search from.
        *
        * @return The position of the first occurrence of the character or sequence of characters.
        */
       std::size_t find(const char* string, std::size_t pos = 0) const;

       /**
        * Replaces the character or sequence of characters with another character or sequence of characters.
        *
        * @param pos The position to start the replacement from.
        * @param count The number of characters to replace.
        * @param strign The character or sequence of characters to replace with.
        */
       void replace(std::size_t pos, std::size_t count, const char* string);

       /**
        * Replaces the character or sequence of characters with another character or sequence of characters.
        *
        * @param pos The position to start the replacement from.
        * @param count The number of characters to replace.
        * @param string The character or sequence of characters to replace with.
        */
       void replace(std::size_t pos, std::size_t count, const std::string string);

       /**
        * Replaces the character or sequence of characters with another character or sequence of characters. 
        *
        * @param pos The position to start the replacement from.
        * @param count The number of characters to replace.
        * @param string The character or sequence of characters to replace with.
        */
       void replace(std::size_t pos, std::size_t count, const String& string);

       /**
        * Substring method that returns the substring of the string.
        *
        * @param pos The position to start the substring from.
        * @param count The number of characters to include in the substring.
        *
        * @return The substring of the string.
        */
       String substring(std::size_t pos, std::size_t count) const;

       /**
        * Begins the read-only iteration over the string.
        *
        * @return The const char* pointer to the beginning of the string.
        */
       const char* begin() const;

       /**
        * Ends the read-only iteration over the string.
        *
        * @return The const char* pointer to the end of the string.
        */
       const char* end() const;

       /**
        * Begins the read-write iteration over the string.
        *
        * @return The char* pointer to the beginning of the string.
        */
       char* begin();

       /**
        * Ends the read-write iteration over the string.
        *
        * @return The char* pointer to the end of the string.
        */
       char* end();

       // TODO: think about rust-like iterators

    private:
        friend bool operator==(const String& lhs, const String& rhs);
        friend bool operator!=(const String& lhs, const String& rhs);
        friend String operator+(const String& lhs, const String& rhs);
        friend String operator+(const String& lhs, const char* rhs);
        friend String operator+(const String& lhs, std::string rhs);

        char* m_string = nullptr;
        unsigned int m_length = 0;
        unsigned int m_capacity = 0;
    };

    /**
     * Add operator that appends two Strings.
     *
     * @param lhs The left hand side String to append.
     * @param rhs The right hand side String to append.
     * @return The String that is the result of the append operation.
     */
    PN_CHAT_EXPORT String operator+(const String& lhs, const String& rhs);

    /**
     * Add operator that appends a String with a const char* string.
     *
     * @param lhs The left hand side String to append.
     * @param rhs The right hand side const char* string to append.
     * @return The String that is the result of the append operation.
     */
    PN_CHAT_EXPORT    String operator+(const String& lhs, const char* rhs);

    /**
     * Add operator that appends a String with a std::string.
     *
     * @param lhs The left hand side String to append.
     * @param rhs The right hand side std::string to append.
     * @return The String that is the result of the append operation.
     */
    PN_CHAT_EXPORT    String operator+(const String& lhs, std::string rhs);

    /**
     * Equality operator that compares two Strings over their values.
     * 
     * @param lhs The left hand side String to compare.
     * @param rhs The right hand side String to compare.
     * @return True if the String values are equal (ptr do not need to be the same), false otherwise.
     */
    PN_CHAT_EXPORT    bool operator==(const String& lhs, const String& rhs);

    /**
     * Inequality operator that compares two Strings over their values.
     * 
     * @param lhs The left hand side String to compare.
     * @param rhs The right hand side String to compare.
     * @return True if the String values are not equal, false otherwise.
     */
    PN_CHAT_EXPORT    bool operator!=(const String& lhs, const String& rhs);
}

#endif /* PN_STRING_H */
