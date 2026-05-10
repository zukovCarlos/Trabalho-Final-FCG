#ifndef LIST_H
#define LIST_H

#include <stdint.h>
#include <stdlib.h>
#include <type_traits>
#include <iterator>

#define LIST_INVALID_INDEX UINT64_MAX

template <typename ElemType, uint64_t BufferSize> struct List
{
    ElemType**      m_buffers;
    size_t          m_size;
    size_t          m_cursor;

    List()
    {
        m_buffers = nullptr;
        m_size = 0;
        m_cursor = 0;
    };

    ~List()
    {
        if (m_size > 0)
        {
            if (!std::is_trivially_destructible_v<ElemType>)
            {
                for (uint64_t elemIndex = 0; elemIndex < m_cursor; elemIndex++)
                {
                    uint64_t bufferIndex = elemIndex / BufferSize;
                    uint64_t bufferOffset = elemIndex % BufferSize;
                    ElemType *elem = m_buffers[bufferIndex] + bufferOffset;
                    elem->~ElemType();
                }
            }

            uint64_t bufferCount = m_size / BufferSize;

            for (uint64_t bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
            {
                free(m_buffers[bufferIndex]);
            }

            free(m_buffers);
        }
    }

    uint64_t addElem(ElemType *elem)
    {
        uint64_t elemIndex = LIST_INVALID_INDEX;

        elemIndex = m_cursor;
        m_cursor++;

        if (elemIndex >= m_size)
        {
            uint64_t bufferCount = m_size / BufferSize;
            m_buffers = (ElemType **)realloc(m_buffers, sizeof(ElemType *) * (bufferCount + 1));
            m_buffers[bufferCount] = (ElemType *)calloc(BufferSize, sizeof(ElemType));
            m_size += BufferSize;
        }

        uint64_t bufferIndex = elemIndex / BufferSize;
        uint64_t bufferOffset = elemIndex % BufferSize;
        ElemType *newElem = m_buffers[bufferIndex] + bufferOffset;

        if constexpr (!std::is_trivially_constructible<ElemType>::value)
        {
            if (elem != nullptr)
            {
                new (newElem) ElemType(*elem);
            }
            else
            {
                new (newElem) ElemType();
            }
        }
        else
        {
            if (elem != nullptr)
            {
                *newElem = *elem;
            }
        }

        /*if (elem != nullptr)
        {
            m_buffers[bufferIndex][bufferOffset] = *elem;
        }
        else
        {
            if (!std::is_trivially_constructible<ElemType>::value)
            {
                ElemType *elemAlloc = m_buffers[bufferIndex] + bufferOffset;
                new (elemAlloc) ElemType();
            }
        }*/

        return elemIndex;
    }

    void insertElemAt(ElemType *elem, uint64_t index)
    {

    }

    void removeElem(uint64_t index)
    {
        if (index < m_cursor)
        {
            uint64_t bufferIndex = index / BufferSize;
            uint64_t bufferOffset = index % BufferSize;
            ElemType *elem = m_buffers[bufferIndex] + bufferOffset;

            if constexpr (!std::is_trivially_destructible_v<ElemType>)
            {
                elem->~ElemType();
            }

            m_cursor--;

            if (index < m_cursor)
            {
                uint64_t lastBufferIndex = m_cursor / BufferSize;
                uint64_t lastBufferOffset = m_cursor % BufferSize;
                *elem = m_buffers[lastBufferIndex][lastBufferOffset];
            }
        }
    }

    //void removeElemAndShift(uint64_t index)
    //{
    //    if (index < m_cursor)
    //    {
    //        uint64_t bufferIndex = index / BufferSize;
    //        uint64_t bufferOffset = index % BufferSize;
    //        ElemType *elem = m_buffers[bufferIndex] + bufferOffset;

    //        if constexpr (!std::is_trivially_destructible_v<ElemType>)
    //        {
    //            elem->~ElemType();
    //        }

    //        uint64_t lastBufferIndex = m_cursor / BufferSize;
    //        uint64_t copySize = BufferSize - bufferOffset;
    //        for (uint64_t curBufferIndex = bufferIndex; curBufferIndex < lastBufferIndex; curBufferIndex++)
    //        {
    //            memcpy(m_buffers[curBufferIndex] + bufferOffset, )
    //        }

    //        //m_cursor--;

    //        /*if (index < m_cursor)
    //        {
    //            uint64_t lastBufferIndex = m_cursor / BufferSize;
    //            uint64_t lastBufferOffset = m_cursor % BufferSize;
    //            *elem = m_buffers[lastBufferIndex][lastBufferOffset];
    //        }*/
    //    }
    //}

    ElemType *getElem(uint64_t index)
    {
        ElemType *elem = nullptr;

        if (index < m_cursor)
        {
            uint64_t bufferIndex = index / BufferSize;
            uint64_t bufferOffset = index % BufferSize;
            elem = m_buffers[bufferIndex] + bufferOffset;
        }
        return elem;
    }

    inline size_t bufferSize()
    {
        return BufferSize;
    }

    inline size_t elemSize()
    {
        return sizeof(ElemType);
    }

    struct iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = ElemType;
        using pointer           = ElemType*;
        using reference         = ElemType&;

        iterator(List<ElemType, BufferSize>& list, uint64_t index) : m_list(list), m_index(index){}

        reference operator*() const
        {
            uint64_t bufferIndex = m_index / BufferSize;
            uint64_t bufferOffset = m_index % BufferSize;
            return m_list.m_buffers[bufferIndex][bufferOffset];
        }

        pointer operator->() const
        {
            uint64_t bufferIndex = m_index / BufferSize;
            uint64_t bufferOffset = m_index % BufferSize;
            return m_list.m_buffers[bufferIndex] + bufferOffset;
        }

        iterator &operator++() {m_index++; return *this;}
        iterator operator++(int) {iterator temp = *this; ++(*this); return temp; }
        iterator &operator--() {m_index--; return *this;}
        iterator operator--(int) { iterator temp = *this; --(*this); return temp; }

        friend bool operator== (const iterator &a, const iterator &b) { return a.m_index == b.m_index; }
        friend bool operator!= (const iterator &a, const iterator &b) { return a.m_index != b.m_index; }

        difference_type operator- (const iterator &b) const { return m_index - b.m_index; }
        iterator& operator-(difference_type offset) { m_index -= offset; return *this; }

        difference_type operator+ (const iterator &b) { return m_index + b.m_index; }
        iterator& operator+ (difference_type offset) { m_index += offset; return *this; }

        iterator& operator=(iterator& other) { m_list = other.m_list; m_index = other.m_index; return *this; }
        bool operator<(const iterator& other) { return m_index < other.m_index; }

        private:
            List<ElemType, BufferSize>&     m_list;
            uint64_t                        m_index;
    };

    iterator begin() { return iterator(*this, 0); }
    iterator end() { return iterator(*this, m_cursor); }
};


#endif