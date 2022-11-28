/**
 *  @file   reader.hpp
 *  @brief  A generic Data Reader class.
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
 * DataReader class
 * Attributes: marketData
 * friend class: DataReaderIterator
 */
class DataReader
{
private:
    std::vector<std::string> marketData;
    std::string line, cell;
    friend class DataReaderIterator;

protected:
    DataReader() {}

    ~DataReader() {
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
        }
    }

    // operator overload on >> to allow iterator to call `readNextEvent`
    friend std::istream& operator>>(std::istream& str, DataReader& DataReader) {
        DataReader.readNextEvent(str);
        return str;
    }
};


/*
* DataReaderIterator
* Description: Iterator for DataReader
*/
class DataReaderIterator {
public:
    typedef std::input_iterator_tag iterator_category;
    typedef std::size_t difference_type;
    typedef DataReader value_type;
    typedef DataReader* pointer;
    typedef DataReader& reference;

    DataReaderIterator(std::istream& str): m_str(str.good() ? &str : NULL) { ++(*this); }

    DataReaderIterator(): m_str(NULL) {}

    // operator overload on ++ to allow post increment 
    DataReaderIterator operator++(int) {
        DataReaderIterator tmp(*this);
        ++(*this);
        return tmp;
    }

    // operator overload on ++ to allow pre increment. 
    // utilized by friend function in parent DataReader class to call `readNextEvent`
    DataReaderIterator& operator++() {
        if (m_str) {
            if (!((*m_str) >> DataReader_)) {
                m_str = NULL;
            }
        }
        return *this;
    }

    // other relevant operator overloads
    DataReader const& operator*() const { return DataReader_; }
    DataReader const* operator->() const { return &DataReader_; }
    bool operator==(DataReaderIterator const& rhs) {
        return ((this == &rhs) || ((this->m_str == NULL) && (rhs.m_str == NULL)));
    }
    bool operator!=(DataReaderIterator const& rhs) {
        return !((*this) == rhs);
    }

private:
    std::istream* m_str;
    DataReader DataReader_;
};
#endif