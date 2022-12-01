/**
 *  @file   mdr.hpp
 *  @brief  A generic MDR class.
 *  @author Vikram Bhattacharjee
 *  @date  2022-11-26
 ***********************************************/

#ifndef _READER__HPP__
#define _READER__HPP__

#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>
#include <string>

 /*
 * MDR class
 * Attributes: marketData
 * friend class: MDRIterator
 */
class MDR
{
private:
    std::vector<std::string> marketData;
    std::string line, cell;
    friend class MDRIterator;

protected:
    MDR() {}

    ~MDR() {
        marketData.clear();
        line.clear();
        cell.clear();
    }

public:
    // operator overload on [] to allow indexing on local container
    std::string const& operator[](std::size_t index) const {
        return marketData[index];
    }

    // readNextEvent reads next row from stream
    void readNextEvent(std::istream& str) {
        std::getline(str, line);
        std::stringstream lineStream(line);
        marketData.clear();
        line.clear();

        while (std::getline(lineStream, cell, ',')) {
            marketData.emplace_back(cell);
            cell.clear();
            break;
        }
    }

    // operator overload on >> to allow iterator to call `readNextEvent`
    friend std::istream& operator>>(std::istream& str, MDR& mdr) {
        mdr.readNextEvent(str);
        return str;
    }
};


/*
* MDRIterator
* Description: Iterator for MDR
*/
class MDRIterator {
public:
    typedef std::input_iterator_tag iterator_category;
    typedef std::size_t difference_type;
    typedef MDR value_type;
    typedef MDR* pointer;
    typedef MDR& reference;

    MDRIterator(std::istream& str): m_str(str.good() ? &str : NULL) { ++(*this); }

    MDRIterator(): m_str(NULL) {}

    // operator overload on ++ to allow post increment 
    MDRIterator operator++(int) {
        MDRIterator tmp(*this);
        ++(*this);
        return tmp;
    }

    // operator overload on ++ to allow pre increment. 
    // utilized by friend function in parent MDR class to call `readNextEvent`
    MDRIterator& operator++() {
        if (m_str) {
            if (!((*m_str) >> mdr_)) {
                m_str = NULL;
            }
        }
        return *this;
    }

    // other relevant operator overloads
    MDR const& operator*() const { return mdr_; }
    MDR const* operator->() const { return &mdr_; }
    bool operator==(MDRIterator const& rhs) {
        return ((this == &rhs) || ((this->m_str == NULL) && (rhs.m_str == NULL)));
    }
    bool operator!=(MDRIterator const& rhs) {
        return !((*this) == rhs);
    }

private:
    std::istream* m_str;
    MDR mdr_;
};
#endif