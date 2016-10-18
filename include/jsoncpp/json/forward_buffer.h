#pragma once

#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <vector>

namespace Json
{

/**
 * Wraps a raw input stream into a buffered forward-iterator interface to
 * allow parsing with backtracking.
 *
 * Iterators of this class mimic a raw character stream interface, with
 * sensible implementations of pointer-arithmetic semantics.  This allows
 * it to slip underneath a string-based parsing implementation by acting
 * like a contiguous character array of maximum size.
 *
 * A buffer may be initialized from a string or general input char stream.
 */
struct ForwardBuffer
{

    /**
     * Initialize a buffer from an input string.
     */
    void
    reset( const std::string &str_input )
    {
        this->owned_stream.reset( new std::istringstream( str_input ) );
        reset( *( this->owned_stream ), str_input.size() );
    }

    /**
     * Initialize a buffer from a character input stream.
     */
    void
    reset( std::istream &stream,
           size_t input_size = std::numeric_limits<size_t>::max() )
    {
        reset( std::istreambuf_iterator<char>( stream ), input_size );
    }

    /**
     * Initialize a buffer from a one-pass character stream iterator.
     */
    void
    reset( std::istreambuf_iterator<char> input,
           size_t input_size = std::numeric_limits<size_t>::max() )
    {
        this->input = input;
        this->input_size = input_size;
        this->buffer.clear();
    }

    struct Iterator;

    /**
     * Re-initialize a buffer, discarding data before a particular buffer
     * position.
     */
    void
    resetTo( Iterator it )
    {
        if ( it == end() )
        {
            this->buffer.clear();
            this->input_size = 0;
            return;
        }

        this->buffer.erase( buffer.begin(), it.raw() );
        if ( this->input_size != std::numeric_limits<size_t>::max() )
            this->input_size -= it.index;
    }

    /**
     * Iterator mimics a character pointer array of configurable size - for
     * unbounded streams it is an array of maximum size.
     */
    struct Iterator
    {

        Iterator() : buffer( 0 ), index( 0 ) {}

        Iterator( int ptr )
            : buffer( reinterpret_cast<ForwardBuffer *>( ptr ) ), index( 0 )
        {
        }

        Iterator( ForwardBuffer *buffer, size_t index )
            : buffer( buffer ), index( index )
        {
        }

        // ptr = 0
        Iterator &
        operator=( int ptr )
        {
            buffer = reinterpret_cast<ForwardBuffer *>( ptr );
            return *this;
        }

        // if (ptr)
        operator bool() const { return buffer != 0; }

        // *ptr
        const char &operator*() const { return buffer->buffer[index]; }

        // ++ptr
        Iterator operator++()
        {
            index = buffer->nextIndex( index );
            return *this;
        }

        // ptr++
        Iterator operator++( int )
        {
            Iterator result( *this );
            ++( *this );
            return result;
        }

        // ptr_a == ptr_b
        bool
        operator==( const Iterator &other ) const
        {
            return index == other.index;
        }

        // ptr_a != ptr_b
        bool
        operator!=( const Iterator &other ) const
        {
            return !( *this == other );
        }

        // ptr_a < ptr_b
        bool
        operator<( const Iterator &other ) const
        {
            return index < other.index;
        }

        // ptr - 1
        Iterator
        operator-( int n ) const
        {
            return Iterator( buffer, index - n );
        }

        // ptr + 1
        Iterator
        operator+( int n ) const
        {
            return Iterator( buffer, index + n );
        }

        // ptr += 1
        Iterator &
        operator+=( int n )
        {
            for ( int i = 0; i < n; ++i ) index = buffer->nextIndex( index );
            return *this;
        }

        // ptr_a - ptr_b
        size_t
        operator-( const Iterator &other ) const
        {
            return index - other.index;
        }

        // ptr[1]
        const char &operator[]( size_t n ) const
        {
            return buffer->at( index + n );
        }

        /**
         * Raw access to the buffer iterator, for std::string
         */
        std::vector<char>::iterator
        raw()
        {
            return buffer->buffer.begin() + index;
        }

        // We buffer sometimes in the background, but that shouldn't
        // affect semantics
        mutable ForwardBuffer *buffer;
        size_t index;
    };

    Iterator
    begin()
    {
        return Iterator( this, !bufferTo( 1 ) ? input_size : 0 );
    }

    Iterator
    end()
    {
        return Iterator( this, input_size );
    }

    const char &
    at( size_t index )
    {
        bufferTo( index + 1 );
        return buffer[index];
    }

    bool
    bufferTo( size_t size )
    {
        while ( buffer.size() < size )
        {
            if ( input == std::istreambuf_iterator<char>() ||
                 buffer.size() == input_size )
            {	
                return false;
            }
            buffer.push_back( *input );
            ++input;
        }
        
        return true;
    }

    /**
     * Increment index, where last index is at input_size and may be
     * non-contiguous.
     */
    size_t
    nextIndex( size_t index )
    {
        if ( index == input_size ) return index;
        return !bufferTo( ( index + 1 ) + 1 ) ? input_size : index + 1;
    }

    std::string
    getBuffered()
    {
        return std::string( buffer.begin(), buffer.end() );
    }

    // Sometimes it's convenient to own the stream
    std::unique_ptr<std::istream> owned_stream;
    std::istreambuf_iterator<char> input;
    size_t input_size;
    std::vector<char> buffer;
};
}