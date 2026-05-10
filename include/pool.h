#ifndef POOL_H
#define POOL_H

#include <stdint.h>
#include <stdlib.h>
#include <type_traits>
#include <iterator>

#define POOL_INVALID_INDEX UINT64_MAX

struct PoolElem
{
    uint64_t m_index;

    bool operator == (const PoolElem& other) { return m_index == other.m_index; };
    bool operator != (const PoolElem& other) { return m_index != other.m_index; };
};

#define POOL_INVALID_ELEM PoolElem{.m_index = POOL_INVALID_INDEX}

template <typename ElemType, uint64_t BufferSize, uint64_t ElemOffset = offsetof(ElemType, m_index)> struct Pool
{
    ElemType**      m_buffers;
    size_t          m_size;
    size_t          m_bufferCount;
    size_t          m_cursor;
    uint64_t*       m_freeStack;
    uint64_t        m_freeStackTop;

    Pool()
    {
        m_buffers = nullptr;
        m_size = 0;
        m_cursor = 0;
        m_bufferCount = 0;
        m_freeStack = nullptr;
        m_freeStackTop = UINT64_MAX;
    };

    ~Pool()
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

                    PoolElem *poolElem = (PoolElem *)((uintptr_t)elem + ElemOffset);
                    if (poolElem->m_index != POOL_INVALID_INDEX)
                    {
                        elem->~ElemType();
                        poolElem->m_index = POOL_INVALID_INDEX;
                    }
                }
            }

            //uint64_t bufferCount = m_size / BufferSize;

            for (uint64_t bufferIndex = 0; bufferIndex < m_bufferCount; bufferIndex++)
            {
                free(m_buffers[bufferIndex]);
            }

            free(m_buffers);
            free(m_freeStack);
        }
    }

    ElemType *addElem(ElemType *elem)
    {
        uint64_t elemIndex = POOL_INVALID_INDEX;

        if (m_freeStackTop != UINT64_MAX)
        {
            elemIndex = m_freeStack[m_freeStackTop];
            m_freeStackTop--;
        }
        else
        {
            elemIndex = m_cursor;
            m_cursor++;

            if (elemIndex >= m_size)
            {
                //uint64_t bufferCount = m_size / BufferSize;
                m_buffers = (ElemType **)realloc(m_buffers, sizeof(ElemType *) * (m_bufferCount + 1));
                m_buffers[m_bufferCount] = (ElemType *)calloc(BufferSize, sizeof(ElemType));
                m_size += BufferSize;
                m_bufferCount++;
                m_freeStack = (uint64_t *)realloc(m_freeStack, sizeof(uint64_t) * m_size);
            }
        }
        uint64_t bufferIndex = elemIndex / BufferSize;
        uint64_t bufferOffset = elemIndex % BufferSize;

        ElemType *newElem = m_buffers[bufferIndex] + bufferOffset;

        if constexpr (!std::is_trivially_constructible<ElemType>::value)
        {
            if constexpr (std::is_copy_constructible<ElemType>::value)
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

        PoolElem *poolElem = (PoolElem *)((uintptr_t)newElem + ElemOffset);
        poolElem->m_index = elemIndex;

        return newElem;
    }

    void removeElem(ElemType *elem)
    {
        if (elem != nullptr && elem->m_index != POOL_INVALID_INDEX)
        {
            PoolElem *poolElem = (PoolElem *)((uintptr_t)elem + ElemOffset);

            if constexpr (!std::is_trivially_destructible_v<ElemType>)
            {
                elem->~ElemType();
            }

            if (poolElem->m_index != POOL_INVALID_INDEX)
            {
                m_freeStackTop++;
                m_freeStack[m_freeStackTop] = poolElem->m_index;
                poolElem->m_index = POOL_INVALID_INDEX;
            }
        }
    }

    ElemType *getElem(uint64_t index)
    {
        ElemType *elem = nullptr;
        
        if (index < m_cursor)
        {
            uint64_t bufferIndex = index / BufferSize;
            uint64_t bufferOffset = index % BufferSize;
            elem = m_buffers[bufferIndex] + bufferOffset;

            PoolElem *poolElem = (PoolElem *)((uintptr_t)elem + ElemOffset);

            if (poolElem->m_index == POOL_INVALID_INDEX)
            {
                elem = nullptr;
            }
        }
        return elem;
    }

    void clear()
    {
        if constexpr (!std::is_trivially_destructible_v<ElemType>)
        {
            for (uint32_t index = 0; index < m_cursor; index++)
            {
                ElemType *elem = m_buffers[index / BufferSize] + (index % BufferSize);

                if (elem->m_index != POOL_INVALID_INDEX)
                {
                    elem->~ElemType();
                }
            }
        }

        m_cursor = 0;
        m_freeStackTop = UINT64_MAX;
    }

    size_t size()
    {
        return m_cursor - (m_freeStackTop + 1);
    }

    struct iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = ElemType;
        using pointer = ElemType*;
        using reference = ElemType&;

        iterator(Pool<ElemType, BufferSize, ElemOffset>& pool, uint64_t index) : m_pool(pool)
        {
            m_index = index;
            m_bufferOffset = index % BufferSize;
            m_bufferIndex = index / BufferSize;
        }

        reference operator*() const
        {
            return m_pool.m_buffers[m_bufferIndex][m_bufferOffset];
        }

        pointer operator->() const
        {
            return m_pool.m_buffers[m_bufferIndex] + m_bufferOffset;
        }

        iterator &operator++() 
        { 
            do
            {
                m_index++;
                m_bufferOffset++;

                if (m_bufferOffset >= BufferSize)
                {
                    m_bufferIndex++;
                    m_bufferOffset = 0;

                    if (m_bufferIndex >= m_pool.m_bufferCount)
                    {
                        m_bufferIndex = m_pool.m_bufferCount;
                        m_index = m_pool.m_cursor;
                        break;
                    }

                }

            }while(m_pool.m_buffers[m_bufferIndex][m_bufferOffset].m_index == POOL_INVALID_INDEX);
            return *this;
        }
        iterator operator++(int) { iterator temp = *this; ++(*this); return temp; }

        iterator &operator--() 
        { 
            do
            {
                m_index--;
                m_bufferOffset--;

                /* true on underflow */
                if (m_bufferOffset >= BufferSize)
                {
                    m_bufferIndex--;
                    m_bufferOffset = 0;

                    if (m_bufferIndex == UINT64_MAX)
                    {
                        m_bufferIndex = 0;
                        m_index = 0;
                        break;
                    }
                }

            } while (m_pool.m_buffers[m_bufferIndex][m_bufferOffset].m_index == POOL_INVALID_INDEX);
            return *this;
        }
        iterator operator--(int) { iterator temp = *this; --(*this); return temp; }

        friend bool operator== (const iterator &a, const iterator &b) { return a.m_index == b.m_index; }
        friend bool operator!= (const iterator &a, const iterator &b) { return a.m_index != b.m_index; }

        difference_type operator- (const iterator &b) const 
        { 
            return m_index - b.m_index;
        }

        iterator& operator-= (difference_type offset) 
        { 
            if (m_index <= offset)
            {
                m_index = 0;
            }
            else
            {
                m_index -= offset;
            }

            m_bufferOffset = m_index % BufferSize;
            m_bufferIndex = m_index / BufferSize;
            return *this; 
        }

        difference_type operator+ (const iterator &b) { return m_index + b.m_index; }

        iterator& operator+= (difference_type offset) 
        { 
            m_index += offset;

            if (m_index >= m_pool.m_cursor)
            {
                m_index = m_pool.m_cursor;
            }

            m_bufferOffset = m_index % BufferSize;
            m_bufferIndex = m_index / BufferSize;

            return *this; 
        }

        iterator& operator=(iterator& other) 
        { 
            m_pool = other.m_pool; 
            m_index = other.m_index;
            m_bufferIndex = other.m_bufferIndex; 
            m_bufferOffset = other.m_bufferOffset;
            return *this; 
        }

        bool operator<(const iterator& other) 
        { 
            return m_index < other.m_index;
        }

    private:
        Pool<ElemType, BufferSize, ElemOffset>&     m_pool;
        uint64_t                                    m_index;
        uint64_t                                    m_bufferOffset;
        uint64_t                                    m_bufferIndex;
    };

    iterator begin() { return iterator(*this, 0); }
    iterator end() { return iterator(*this, m_cursor); }
};


#endif
