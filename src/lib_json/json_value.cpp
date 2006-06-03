#include <json/value.h>
#include <json/writer.h>
#include <utility>
#include "assert.h"
#ifdef JSON_USE_CPPTL
# include <cpptl/conststring.h>
# include <cpptl/enumerator.h>
#endif
#include <stddef.h>    // size_t

#define JSON_ASSERT_UNREACHABLE assert( false )
#define JSON_ASSERT( condition ) assert( condition );  // @todo <= change this into an exception throw
#define JSON_ASSERT_MESSAGE( condition, message ) assert( condition &&  message );  // @todo <= change this into an exception throw

namespace Json {

const Value Value::null;
const Value::Int Value::minInt = Value::Int( ~(Value::UInt(-1)/2) );
const Value::Int Value::maxInt = Value::Int( Value::UInt(-1)/2 );
const Value::UInt Value::maxUInt = Value::UInt(-1);

// A "safe" implementation of strdup. Allow null pointer to be passed. 
// Also avoid warning on msvc80.

inline char *safeStringDup( const char *czstring )
{
   if ( czstring )
   {
      const size_t length = (unsigned int)( strlen(czstring) + 1 );
      char *newString = static_cast<char *>( malloc( length ) );
      memcpy( newString, czstring, length );
      return newString;
   }
   return 0;
}

inline char *safeStringDup( const std::string &str )
{
   if ( !str.empty() )
   {
      const size_t length = str.length();
      char *newString = static_cast<char *>( malloc( length + 1 ) );
      memcpy( newString, str.c_str(), length );
      newString[length] = 0;
      return newString;
   }
   return 0;
}

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class ValueIteratorBase
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

ValueIteratorBase::ValueIteratorBase()
{
}


ValueIteratorBase::ValueIteratorBase( const Value::ObjectValues::iterator &current )
   : current_( current )
{
}


Value &
ValueIteratorBase::deref() const
{
   return current_->second;
}


void 
ValueIteratorBase::increment()
{
   ++current_;
}


void 
ValueIteratorBase::decrement()
{
   --current_;
}


ValueIteratorBase::difference_type 
ValueIteratorBase::computeDistance( const SelfType &other ) const
{
# ifdef JSON_USE_CPPTL_SMALLMAP
   return current_ - other.current_;
# else
   return difference_type( std::distance( current_, other.current_ ) );
# endif
}


bool 
ValueIteratorBase::isEqual( const SelfType &other ) const
{
   return current_ == other.current_;
}


void 
ValueIteratorBase::copy( const SelfType &other )
{
   current_ = other.current_;
}


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class ValueConstIterator
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

ValueConstIterator::ValueConstIterator()
{
}


ValueConstIterator::ValueConstIterator( const Value::ObjectValues::iterator &current )
   : ValueIteratorBase( current )
{
}

ValueConstIterator &
ValueConstIterator::operator =( const ValueIteratorBase &other )
{
   copy( other );
   return *this;
}


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class ValueIterator
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

ValueIterator::ValueIterator()
{
}


ValueIterator::ValueIterator( const Value::ObjectValues::iterator &current )
   : ValueIteratorBase( current )
{
}

ValueIterator::ValueIterator( const ValueConstIterator &other )
   : ValueIteratorBase( other )
{
}

ValueIterator::ValueIterator( const ValueIterator &other )
   : ValueIteratorBase( other )
{
}

ValueIterator &
ValueIterator::operator =( const SelfType &other )
{
   copy( other );
   return *this;
}



// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class Value::CommentInfo
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////


Value::CommentInfo::CommentInfo()
   : comment_( 0 )
{
}

Value::CommentInfo::~CommentInfo()
{
   if ( comment_ )
      free( comment_ );
}


void 
Value::CommentInfo::setComment( const char *text )
{
   if ( comment_ )
      free( comment_ );
   comment_ = safeStringDup( text );
}


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class Value::CZString
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

// Notes: index_ indicates if the string was allocated when
// a string is stored.

Value::CZString::CZString( int index )
   : cstr_( 0 )
   , index_( index )
{
}

Value::CZString::CZString( const char *cstr, DuplicationPolicy allocate )
   : cstr_( allocate == duplicate ? safeStringDup(cstr) : cstr )
   , index_( allocate )
{
}

Value::CZString::CZString( const CZString &other )
   : cstr_( other.index_ != noDuplication &&  other.cstr_ != 0 ? safeStringDup( other.cstr_ )
                                                               : other.cstr_ )
   , index_( other.cstr_ ? (other.index_ == noDuplication ? noDuplication : duplicate)
                         : other.index_ )
{
}

Value::CZString::~CZString()
{
   if ( cstr_  &&  index_ == duplicate )
      free( const_cast<char *>( cstr_ ) );
}

void 
Value::CZString::swap( CZString &other )
{
   std::swap( cstr_, other.cstr_ );
   std::swap( index_, other.index_ );
}

Value::CZString &
Value::CZString::operator =( const CZString &other )
{
   CZString temp( other );
   swap( temp );
   return *this;
}

bool 
Value::CZString::operator<( const CZString &other ) const 
{
   if ( cstr_ )
      return strcmp( cstr_, other.cstr_ ) < 0;
   return index_ < other.index_;
}

bool 
Value::CZString::operator==( const CZString &other ) const 
{
   if ( cstr_ )
      return strcmp( cstr_, other.cstr_ ) == 0;
   return index_ == other.index_;
}


int 
Value::CZString::index() const
{
   return index_;
}


const char *
Value::CZString::c_str() const
{
   return cstr_;
}


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class Value::Value
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////


Value::Value( ValueType type )
   : type_( type )
   , comments_( 0 )
   , allocated_( 0 )
{
   switch ( type )
   {
   case nullValue:
      break;
   case intValue:
   case uintValue:
      value_.int_ = 0;
      break;
   case realValue:
      value_.real_ = 0.0;
      break;
   case stringValue:
      value_.string_ = 0;
      break;
   case arrayValue:
   case objectValue:
      value_.map_ = new ObjectValues();
      break;
   case booleanValue:
      value_.bool_ = false;
      break;
   default:
      JSON_ASSERT_UNREACHABLE;
   }
}


Value::Value( Int value )
   : type_( intValue )
   , comments_( 0 )
{
   value_.int_ = value;
}


Value::Value( UInt value )
   : type_( uintValue )
   , comments_( 0 )
{
   value_.uint_ = value;
}

Value::Value( double value )
   : type_( realValue )
   , comments_( 0 )
{
   value_.real_ = value;
}

Value::Value( const char *value )
   : type_( stringValue )
   , allocated_( true )
   , comments_( 0 )
{
   value_.string_ = safeStringDup( value );
}

Value::Value( const std::string &value )
   : type_( stringValue )
   , allocated_( true )
   , comments_( 0 )
{
   value_.string_ = safeStringDup( value );

}
# ifdef JSON_USE_CPPTL
Value::Value( const CppTL::ConstString &value )
   : type_( stringValue )
   , allocated_( true )
   , comments_( 0 )
{
   value_.string_ = safeStringDup( value );
}
# endif

Value::Value( bool value )
   : type_( booleanValue )
   , comments_( 0 )
{
   value_.bool_ = value;
}


Value::Value( const Value &other )
   : type_( other.type_ )
   , comments_( 0 )
{
   switch ( type_ )
   {
   case nullValue:
   case intValue:
   case uintValue:
   case realValue:
   case booleanValue:
      value_ = other.value_;
      break;
   case stringValue:
      if ( other.value_.string_ )
      {
         value_.string_ = safeStringDup( other.value_.string_ );
         allocated_ = true;
      }
      else
         value_.string_ = 0;
      break;
   case arrayValue:
   case objectValue:
      value_.map_ = new ObjectValues( *other.value_.map_ );
      // @todo for each, reset parent...
      break;
   default:
      JSON_ASSERT_UNREACHABLE;
   }
   if ( other.comments_ )
   {
      comments_ = new CommentInfo[numberOfCommentPlacement];
      for ( int comment =0; comment < numberOfCommentPlacement; ++comment )
      {
         const CommentInfo &otherComment = other.comments_[comment];
         if ( otherComment.comment_ )
            comments_[comment].setComment( otherComment.comment_ );
      }
   }
}


Value::~Value()
{
   switch ( type_ )
   {
   case nullValue:
   case intValue:
   case uintValue:
   case realValue:
   case booleanValue:
      break;
   case stringValue:
      if ( allocated_ )
         free( value_.string_ );
      break;
   case arrayValue:
   case objectValue:
      delete value_.map_;
      break;
   default:
      JSON_ASSERT_UNREACHABLE;
   }

   if ( comments_ )
      delete[] comments_;
}

Value &
Value::operator=( const Value &other )
{
   Value temp( other );
   swap( temp );
   return *this;
}

void 
Value::swap( Value &other )
{
   ValueType temp = type_;
   type_ = other.type_;
   other.type_ = temp;
   std::swap( value_, other.value_ );
   bool temp2 = allocated_;
   allocated_ = other.allocated_;
   other.allocated_ = temp2;
}

ValueType 
Value::type() const
{
   return type_;
}


int 
Value::compare( const Value &other )
{
   /*
   int typeDelta = other.type_ - type_;
   switch ( type_ )
   {
   case nullValue:

      return other.type_ == type_;
   case intValue:
      if ( other.type_.isNumeric()
   case uintValue:
   case realValue:
   case booleanValue:
      break;
   case stringValue,
      free( value_.string_ );
      break;
   case arrayValue:
      delete value_.array_;
      break;
   case objectValue:
      delete value_.map_;
   default:
      JSON_ASSERT_UNREACHABLE;
   }
   */
   return 0;  // unreachable
}

bool 
Value::operator <( const Value &other ) const
{
   int typeDelta = type_ - other.type_;
   if ( typeDelta )
      return typeDelta < 0 ? true : false;
   switch ( type_ )
   {
   case nullValue:
      return false;
   case intValue:
      return value_.int_ < other.value_.int_;
   case uintValue:
      return value_.uint_ < other.value_.uint_;
   case realValue:
      return value_.real_ < other.value_.real_;
   case booleanValue:
      return value_.bool_ < other.value_.bool_;
   case stringValue:
      return ( value_.string_ == 0  &&  other.value_.string_ )
             || ( other.value_.string_  
                  &&  value_.string_  
                  && strcmp( value_.string_, other.value_.string_ ) < 0 );
   case arrayValue:
   case objectValue:
      {
         int delta = int( value_.map_->size() - other.value_.map_->size() );
         if ( delta )
            return delta < 0;
         return (*value_.map_) < (*other.value_.map_);
      }
   default:
      JSON_ASSERT_UNREACHABLE;
   }
   return 0;  // unreachable
}

bool 
Value::operator <=( const Value &other ) const
{
   return !(other > *this);
}

bool 
Value::operator >=( const Value &other ) const
{
   return !(*this < other);
}

bool 
Value::operator >( const Value &other ) const
{
   return other < *this;
}

bool 
Value::operator ==( const Value &other ) const
{
   if ( type_ != other.type_ )
      return false;
   switch ( type_ )
   {
   case nullValue:
      return true;
   case intValue:
      return value_.int_ == other.value_.int_;
   case uintValue:
      return value_.uint_ == other.value_.uint_;
   case realValue:
      return value_.real_ == other.value_.real_;
   case booleanValue:
      return value_.bool_ == other.value_.bool_;
   case stringValue:
      return ( value_.string_ == other.value_.string_ )
             || ( other.value_.string_  
                  &&  value_.string_  
                  && strcmp( value_.string_, other.value_.string_ ) == 0 );
   case arrayValue:
   case objectValue:
      return value_.map_->size() == other.value_.map_->size()
             && (*value_.map_) == (*other.value_.map_);
   default:
      JSON_ASSERT_UNREACHABLE;
   }
   return 0;  // unreachable
}

bool 
Value::operator !=( const Value &other ) const
{
   return !( *this == other );
}

const char *
Value::asCString() const
{
   JSON_ASSERT( type_ == stringValue );
   return value_.string_;
}


std::string 
Value::asString() const
{
   switch ( type_ )
   {
   case nullValue:
      return "";
   case stringValue:
      return value_.string_ ? value_.string_ : "";
   case booleanValue:
      return value_.bool_ ? "true" : "false";
   case intValue:
   case uintValue:
   case realValue:
   case arrayValue:
   case objectValue:
      JSON_ASSERT( "Type is not convertible to double" && false );
   default:
      JSON_ASSERT_UNREACHABLE;
   }
   return ""; // unreachable
}

# ifdef JSON_USE_CPPTL
CppTL::ConstString 
Value::asConstString() const
{
   return CppTL::ConstString( asString().c_str() );
}
# endif

Value::Int 
Value::asInt() const
{
   switch ( type_ )
   {
   case nullValue:
      return 0;
   case intValue:
      return value_.int_;
   case uintValue:
      JSON_ASSERT( value_.uint_ < maxInt  &&  "integer out of signed integer range" );
      return value_.uint_;
   case realValue:
      JSON_ASSERT( value_.real_ >= minInt  &&  value_.real_ <= maxInt &&  "Real out of signed integer range" );
      return Int( value_.real_ );
   case booleanValue:
      return value_.bool_ ? 1 : 0;
   case stringValue:
   case arrayValue:
   case objectValue:
      JSON_ASSERT( "Type is not convertible to double" && false );
   default:
      JSON_ASSERT_UNREACHABLE;
   }
   return 0; // unreachable;
}

Value::UInt 
Value::asUInt() const
{
   switch ( type_ )
   {
   case nullValue:
      return 0;
   case intValue:
      JSON_ASSERT( value_.int_ >= 0  &&  "Negative integer can not be converted to unsigned integer" );
      return value_.int_;
   case uintValue:
      return value_.uint_;
   case realValue:
      JSON_ASSERT( value_.real_ >= 0  &&  value_.real_ <= maxUInt &&  "Real out of unsigned integer range" );
      return UInt( value_.real_ );
   case booleanValue:
      return value_.bool_ ? 1 : 0;
   case stringValue:
   case arrayValue:
   case objectValue:
      JSON_ASSERT( "Type is not convertible to double" && false );
   default:
      JSON_ASSERT_UNREACHABLE;
   }
   return 0; // unreachable;
}

double 
Value::asDouble() const
{
   switch ( type_ )
   {
   case nullValue:
      return 0.0;
   case intValue:
      return value_.int_;
   case uintValue:
      return value_.uint_;
   case realValue:
      return value_.real_;
   case booleanValue:
      return value_.bool_ ? 1.0 : 0.0;
   case stringValue:
   case arrayValue:
   case objectValue:
      JSON_ASSERT( "Type is not convertible to double" && false );
   default:
      JSON_ASSERT_UNREACHABLE;
   }
   return 0; // unreachable;
}

bool 
Value::asBool() const
{
   switch ( type_ )
   {
   case nullValue:
      return false;
   case intValue:
   case uintValue:
      return value_.int_ != 0;
   case realValue:
      return value_.real_ != 0.0;
   case booleanValue:
      return value_.bool_;
   case stringValue:
      return value_.string_  &&  value_.string_[0] != 0;
   case arrayValue:
   case objectValue:
      return value_.map_->size() != 0;
   default:
      JSON_ASSERT_UNREACHABLE;
   }
   return false; // unreachable;
}


bool 
Value::isConvertibleTo( ValueType other ) const
{
   switch ( type_ )
   {
   case nullValue:
      return true;
   case intValue:
      return ( other == nullValue  &&  value_.int_ == 0 )
             || other == intValue
             || ( other == uintValue  && value_.int_ >= 0 )
             || other == realValue
             || other == stringValue
             || other == booleanValue;
   case uintValue:
      return ( other == nullValue  &&  value_.uint_ == 0 )
             || ( other == intValue  && value_.uint_ <= maxInt )
             || other == uintValue
             || other == realValue
             || other == stringValue
             || other == booleanValue;
   case realValue:
      return ( other == nullValue  &&  value_.real_ == 0.0 )
             || ( other == intValue  &&  value_.real_ >= minInt  &&  value_.real_ <= maxInt )
             || ( other == uintValue  &&  value_.real_ >= 0  &&  value_.real_ <= maxUInt )
             || other == realValue
             || other == stringValue
             || other == booleanValue;
   case booleanValue:
      return ( other == nullValue  &&  value_.bool_ == false )
             || other == intValue
             || other == uintValue
             || other == realValue
             || other == stringValue
             || other == booleanValue;
   case stringValue:
      return other == stringValue
             || ( other == nullValue  &&  (!value_.string_  ||  value_.string_[0] == 0) );
   case arrayValue:
      return other == arrayValue
             ||  ( other == nullValue  &&  value_.map_->size() == 0 );
   case objectValue:
      return other == objectValue
             ||  ( other == nullValue  &&  value_.map_->size() == 0 );
   default:
      JSON_ASSERT_UNREACHABLE;
   }
   return false; // unreachable;
}


/// Number of values in array or object
Value::UInt 
Value::size() const
{
   switch ( type_ )
   {
   case nullValue:
   case intValue:
   case uintValue:
   case realValue:
   case booleanValue:
   case stringValue:
      return 0;
   case arrayValue:  // size of the array is highest index + 1
      if ( !value_.map_->empty() )
      {
         ObjectValues::const_iterator itLast = value_.map_->end();
         --itLast;
         return (*itLast).first.index()+1;
      }
      return 0;
   case objectValue:
      return Int( value_.map_->size() );
   default:
      JSON_ASSERT_UNREACHABLE;
   }
   return 0; // unreachable;
}


void 
Value::clear()
{
   JSON_ASSERT( type_ == nullValue  ||  type_ == arrayValue  || type_ == objectValue );

   switch ( type_ )
   {
   case arrayValue:  // size of the array is highest index + 1
   case objectValue:
      value_.map_->clear();
   default:
      break;
   }
}

void 
Value::resize( UInt newSize )
{
   JSON_ASSERT( type_ == nullValue  ||  type_ == arrayValue );
   if ( type_ == nullValue )
      *this = Value( arrayValue );
   UInt oldSize = size();
   if ( newSize == 0 )
      clear();
   else if ( newSize > oldSize )
      (*this)[ newSize - 1 ];
   else
   {
      for ( UInt index = newSize; index < oldSize; ++index )
         value_.map_->erase( index );
      assert( size() == newSize );
   }
}


Value &
Value::operator[]( UInt index )
{
   JSON_ASSERT( type_ == nullValue  ||  type_ == arrayValue );
   if ( type_ == nullValue )
      *this = Value( arrayValue );
   CZString key( index );
   ObjectValues::iterator it = value_.map_->lower_bound( key );
   if ( it != value_.map_->end()  &&  (*it).first == key )
      return (*it).second;

   ObjectValues::value_type defaultValue( key, null );
   it = value_.map_->insert( it, defaultValue );
   return (*it).second;
}


const Value &
Value::operator[]( UInt index ) const
{
   JSON_ASSERT( type_ == nullValue  ||  type_ == arrayValue );
   if ( type_ == nullValue )
      return null;
   CZString key( index );
   ObjectValues::const_iterator it = value_.map_->find( key );
   if ( it == value_.map_->end() )
      return null;
   return (*it).second;
}


Value &
Value::operator[]( const char *key )
{
   JSON_ASSERT( type_ == nullValue  ||  type_ == objectValue );
   if ( type_ == nullValue )
      *this = Value( objectValue );
   CZString actualKey( key, CZString::duplicateOnCopy );
   ObjectValues::iterator it = value_.map_->lower_bound( actualKey );
   if ( it != value_.map_->end()  &&  (*it).first == actualKey )
      return (*it).second;

   ObjectValues::value_type defaultValue( actualKey, null );
   it = value_.map_->insert( it, defaultValue );
   Value &value = (*it).second;
   return value;
}


Value 
Value::get( UInt index, 
            const Value &defaultValue ) const
{
   const Value *value = &((*this)[index]);
   return value == &null ? defaultValue : *value;
}


bool 
Value::isValidIndex( UInt index ) const
{
   return index < size();
}



const Value &
Value::operator[]( const char *key ) const
{
   JSON_ASSERT( type_ == nullValue  ||  type_ == objectValue );
   if ( type_ == nullValue )
      return null;
   CZString actualKey( key, CZString::noDuplication );
   ObjectValues::const_iterator it = value_.map_->find( actualKey );
   if ( it == value_.map_->end() )
      return null;
   return (*it).second;
}


Value &
Value::operator[]( const std::string &key )
{
   return (*this)[ key.c_str() ];
}


const Value &
Value::operator[]( const std::string &key ) const
{
   return (*this)[ key.c_str() ];
}


# ifdef JSON_USE_CPPTL
Value &
Value::operator[]( const CppTL::ConstString &key )
{
   return (*this)[ key.c_str() ];
}


const Value &
Value::operator[]( const CppTL::ConstString &key ) const
{
   return (*this)[ key.c_str() ];
}
# endif


Value &
Value::append( const Value &value )
{
   return (*this)[size()] = value;
}


Value 
Value::get( const char *key, 
            const Value &defaultValue ) const
{
   const Value *value = &((*this)[key]);
   return value == &null ? defaultValue : *value;
}


Value 
Value::get( const std::string &key,
            const Value &defaultValue ) const
{
   return get( key.c_str(), defaultValue );
}


# ifdef JSON_USE_CPPTL
Value 
Value::get( const CppTL::ConstString &key,
            const Value &defaultValue ) const
{
   return get( key.c_str(), defaultValue );
}
# endif

bool 
Value::isMember( const char *key ) const
{
   const Value *value = &((*this)[key]);
   return value != &null;
}


bool 
Value::isMember( const std::string &key ) const
{
   return isMember( key.c_str() );
}


# ifdef JSON_USE_CPPTL
bool 
Value::isMember( const CppTL::ConstString &key ) const
{
   return isMember( key.c_str() );
}
#endif

Value::Members 
Value::getMemberNames() const
{
   JSON_ASSERT( type_ == nullValue  ||  type_ == objectValue );
   Members members;
   members.reserve( value_.map_->size() );
   ObjectValues::const_iterator it = value_.map_->begin();
   ObjectValues::const_iterator itEnd = value_.map_->end();
   for ( ; it != itEnd; ++it )
      members.push_back( std::string( (*it).first.c_str() ) );
   return members;
}

# ifdef JSON_USE_CPPTL
EnumMemberNames
Value::enumMemberNames() const
{
   if ( type_ == objectValue )
   {
      return CppTL::Enum::any(  CppTL::Enum::transform(
         CppTL::Enum::keys( *(value_.map_), CppTL::Type<const CZString &>() ),
         MemberNamesTransform() ) );
   }
   return EnumMemberNames();
}


EnumValues 
Value::enumValues() const
{
   if ( type_ == objectValue  ||  type_ == arrayValue )
      return CppTL::Enum::anyValues( *(value_.map_), 
                                     CppTL::Type<const Value &>() );
   return EnumValues();
}

# endif


bool 
Value::isBool() const
{
   return type_ == booleanValue;
}


bool 
Value::isInt() const
{
   return type_ == intValue;
}


bool 
Value::isUInt() const
{
   return type_ == uintValue;
}


bool 
Value::isIntegral() const
{
   return type_ == intValue  
          ||  type_ == uintValue  
          ||  type_ == booleanValue;
}


bool 
Value::isDouble() const
{
   return type_ == realValue;
}


bool 
Value::isNumeric() const
{
   return isIntegral() || isDouble();
}


bool 
Value::isString() const
{
   return type_ == stringValue;
}


bool 
Value::isArray() const
{
   return type_ == nullValue  ||  type_ == arrayValue;
}


bool 
Value::isObject() const
{
   return type_ == nullValue  ||  type_ == objectValue;
}


void 
Value::setComment( const char *comment,
                   CommentPlacement placement )
{
   if ( !comments_ )
      comments_ = new CommentInfo[numberOfCommentPlacement];
   comments_[placement].setComment( comment );
}


void 
Value::setComment( const std::string &comment,
                   CommentPlacement placement )
{
   setComment( comment.c_str(), placement );
}


bool 
Value::hasComment( CommentPlacement placement ) const
{
   return comments_ != 0  &&  comments_[placement].comment_ != 0;
}

std::string 
Value::getComment( CommentPlacement placement ) const
{
   if ( hasComment(placement) )
      return comments_[placement].comment_;
   return "";
}


std::string 
Value::toStyledString() const
{
   StyledWriter writer;
   return writer.write( *this );
}


Value::const_iterator 
Value::begin() const
{
   switch ( type_ )
   {
   case arrayValue:
   case objectValue:
      if ( value_.map_ )
         return const_iterator( value_.map_->begin() );
      // fall through default if no valid map
   default:
      return const_iterator();
   }
}

Value::const_iterator 
Value::end() const
{
   switch ( type_ )
   {
   case arrayValue:
   case objectValue:
      if ( value_.map_ )
         return const_iterator( value_.map_->end() );
      // fall through default if no valid map
   default:
      return const_iterator();
   }
}


Value::iterator 
Value::begin()
{
   switch ( type_ )
   {
   case arrayValue:
   case objectValue:
      if ( value_.map_ )
         return iterator( value_.map_->begin() );
      // fall through default if no valid map
   default:
      return iterator();
   }
}

Value::iterator 
Value::end()
{
   switch ( type_ )
   {
   case arrayValue:
   case objectValue:
      if ( value_.map_ )
         return iterator( value_.map_->end() );
      // fall through default if no valid map
   default:
      return iterator();
   }
}


// class PathArgument
// //////////////////////////////////////////////////////////////////

PathArgument::PathArgument()
   : kind_( kindNone )
{
}


PathArgument::PathArgument( Value::UInt index )
   : kind_( kindIndex )
   , index_( index )
{
}


PathArgument::PathArgument( const char *key )
   : kind_( kindKey )
   , key_( key )
{
}


PathArgument::PathArgument( const std::string &key )
   : kind_( kindKey )
   , key_( key.c_str() )
{
}

// class Path
// //////////////////////////////////////////////////////////////////

Path::Path( const std::string &path,
            const PathArgument &a1,
            const PathArgument &a2,
            const PathArgument &a3,
            const PathArgument &a4,
            const PathArgument &a5 )
{
   InArgs in;
   in.push_back( &a1 );
   in.push_back( &a2 );
   in.push_back( &a3 );
   in.push_back( &a4 );
   in.push_back( &a5 );
   makePath( path, in );
}


void 
Path::makePath( const std::string &path,
                const InArgs &in )
{
   const char *current = path.c_str();
   const char *end = current + path.length();
   InArgs::const_iterator itInArg = in.begin();
   while ( current != end )
   {
      if ( *current == '[' )
      {
         ++current;
         if ( *current == '%' )
            addPathInArg( path, in, itInArg, PathArgument::kindIndex );
         else
         {
            Value::UInt index = 0;
            for ( ; current != end && *current >= '0'  &&  *current <= '9'; ++current )
               index = index * 10 + Value::UInt(*current - '0');
            args_.push_back( index );
         }
         if ( current == end  ||  *current++ != ']' )
            invalidPath( path, int(current - path.c_str()) );
      }
      else if ( *current == '%' )
      {
         addPathInArg( path, in, itInArg, PathArgument::kindKey );
         ++current;
      }
      else if ( *current == '.' )
      {
         ++current;
      }
      else
      {
         const char *beginName = current;
         while ( current != end  &&  !strchr( "[.", *current ) )
            ++current;
         args_.push_back( std::string( beginName, current ) );
      }
   }
}


void 
Path::addPathInArg( const std::string &path, 
                    const InArgs &in, 
                    InArgs::const_iterator &itInArg, 
                    PathArgument::Kind kind )
{
   if ( itInArg == in.end() )
   {
      // Error: missing argument %d
   }
   else if ( (*itInArg)->kind_ != kind )
   {
      // Error: bad argument type
   }
   else
   {
      args_.push_back( **itInArg );
   }
}


void 
Path::invalidPath( const std::string &path, 
                   int location )
{
   // Error: invalid path.
}


const Value &
Path::resolve( const Value &root ) const
{
   const Value *node = &root;
   for ( Args::const_iterator it = args_.begin(); it != args_.end(); ++it )
   {
      const PathArgument &arg = *it;
      if ( arg.kind_ == PathArgument::kindIndex )
      {
         if ( !node->isArray()  ||  node->isValidIndex( arg.index_ ) )
         {
            // Error: unable to resolve path (array value expected at position...
         }
         node = &((*node)[arg.index_]);
      }
      else if ( arg.kind_ == PathArgument::kindKey )
      {
         if ( !node->isObject() )
         {
            // Error: unable to resolve path (object value expected at position...)
         }
         node = &((*node)[arg.key_]);
         if ( node == &Value::null )
         {
            // Error: unable to resolve path (object has no member named '' at position...)
         }
      }
   }
   return *node;
}


Value 
Path::resolve( const Value &root, 
               const Value &defaultValue ) const
{
   const Value *node = &root;
   for ( Args::const_iterator it = args_.begin(); it != args_.end(); ++it )
   {
      const PathArgument &arg = *it;
      if ( arg.kind_ == PathArgument::kindIndex )
      {
         if ( !node->isArray()  ||  node->isValidIndex( arg.index_ ) )
            return defaultValue;
         node = &((*node)[arg.index_]);
      }
      else if ( arg.kind_ == PathArgument::kindKey )
      {
         if ( !node->isObject() )
            return defaultValue;
         node = &((*node)[arg.key_]);
         if ( node == &Value::null )
            return defaultValue;
      }
   }
   return *node;
}


Value &
Path::make( Value &root ) const
{
   Value *node = &root;
   for ( Args::const_iterator it = args_.begin(); it != args_.end(); ++it )
   {
      const PathArgument &arg = *it;
      if ( arg.kind_ == PathArgument::kindIndex )
      {
         if ( !node->isArray() )
         {
            // Error: node is not an array at position ...
         }
         node = &((*node)[arg.index_]);
      }
      else if ( arg.kind_ == PathArgument::kindKey )
      {
         if ( !node->isObject() )
         {
            // Error: node is not an object at position...
         }
         node = &((*node)[arg.key_]);
      }
   }
   return *node;
}


} // namespace Json
