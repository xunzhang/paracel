/**
 * @mainpage Extended STL string
 * @author Keenan Tims - ktims@gotroot.ca
 * @version 0.2
 * @date 2005-04-17
 * @section desc Description
 * ext_string aims to provide a portable, bug-free implementation of many useful extensions to the
 * standard STL string class.  These extensions are commonly available among higher-level languages
 * such as Perl and Python, but C++ programmers are generally left on their own when it comes to
 * basic string processing.  By extending the STL's string, we can provide a drop-in replacement for STL
 * strings with the greater functionality of higher-level languages.
 *
 * The primary goal of this library is to make the STL string class more usable to programmers that
 * are doing simple string manipulation on a small scale.  Due to the usability goals of this class,
 * many actions will be inefficiently implemented for the sake of ease of use.  Some of this is
 * mitigated somewhat by doing modification in-place, however many unnecessary copies of data are
 * created by some methods, and the vector-returning methods are inefficient in that they copy the
 * substrings into the vector, then return a copy of the vector.  This would be much more efficient
 * as an iterator model.
 *
 *
 * @section feat Features
 *
 * @li Fully based on the STL, ext_string provides a superset of std::string methods
 * @li String splitting (tokenizing), on a character, a string, or whitespace
 * @li Replacement of substrings or characters with another string or character
 * @li String case operations (check, adjust)
 * @li Integer conversion
 * @li Fully open-source under a BSD-like license for use in any product
 *
 * @if web
 * @section download Downloads
 *
 * Downloads are provided in tar.gz and zip formats containing this documentation, the header file,
 * and the library's changelog.  The latest version of ext_string is 0.2, released on April 17,
 * 2005.
 *
 * @li<a href="files/ext_string-0.2.tar.gz">ext_string-0.2.tar.gz</a>
 * @li<a href="files/ext_string-0.2.zip">ext_string-0.2.zip</a>
 * @li <small><a href="files/ext_string-0.1.tar.gz">ext_string-0.1.tar.gz</a></small>
 * @li <small><a href="files/ext_string-0.1.zip">ext_string-0.1.zip</a></small>
 *
 * @section changelog Changelog
 *
 * The changelog is viewable online <a href="files/CHANGELOG">here</a>
 *
 * @endif
 *
 * @section notes Notes/Limitations
 * @li Copying all the substrings into a vector for the substring methods is pretty inefficient,
 * both for space and time.  It would be more prudent to model an iterator to split the string based
 * on the specified parameters, but this is more difficult to implement and more cumbersome to use.
 * Performance is not the main goal of this library, usability is, thus the tradeoff is deemed to be
 * acceptable.
 * @li References are not used too aptly in this class.  Some performance tuning could be done to
 * minimize unnecessary data copying.
 * @li The basic methods of std::string aren't overridden by this class, thus assigning the return
 * value of eg. string::insert() to an ext_string instance will make an unnecessary string object
 * which is then copy-constructed to an ext_string (I believe, internal workings of inheritance and
 * polymorphism in C++ are somewhat beyond my experience).  These methods should be wrapped by
 * ext_string to return the proper type.
 *
 *
 * @section related Related Documentation
 * 
 * @li SGI's STL string reference: http://www.sgi.com/tech/stl/basic_string.html
 *
 *
 * @section license License
 * 
 * Copyright (c) 2005, Keenan Tims
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 * @li Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * @li Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided with
 * the distribution.
 * @li Neither the name of the Extended STL String project nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * 
 */

#ifndef _EXT_STRING_H
#define _EXT_STRING_H

#include <string>
#include <vector>
#include <iostream>

namespace std
{

	/**
	 * An extension of STL's string providing additional functionality that is often availiable in
	 * higher-level languages such as Python.
	 */
	class ext_string : public string
	{
		public:
			/**
			 * Default constructor
			 *
			 * Constructs an empty ext_string ("")
			 */
			ext_string() : string() { }

			/**
			 * Duplicate the STL string copy constructor
			 *
			 * @param[in] s   The string to copy
			 * @param[in] pos The starting position in the string to copy from
			 * @param[in] n   The number of characters to copy
			 */
			ext_string(const string &s, size_type pos = 0, size_type n = npos) : string(s, pos, npos) { }

			/**
			 * Construct an ext_string from a null-terminated character array
			 *
			 * @param[in] s The character array to copy into the new string
			 */
			ext_string(const value_type *s) : string(s) { }

			/**
			 * Construct an ext_string from a character array and a length
			 *
			 * @param[in] s The character array to copy into the new string
			 * @param[in] n The number of characters to copy
			 */
			ext_string(const value_type *s, size_type n) : string(s, n) { }

			/**
			 * Create an ext_string with @p n copies of @p c
			 *
			 * @param[in] n The number of copies
			 * @param[in] c The character to copy @p n times
			 */
			ext_string(size_type n, value_type c) : string(n, c) { }

			/**
			 * Create a string from a range
			 *
			 * @param[in] first The first element to copy in
			 * @param[in] last  The last element to copy in
			 */
			template <class InputIterator>
				ext_string(InputIterator first, InputIterator last) : string(first, last) { }

			/**
			 * The destructor
			 */
			~ext_string() { }

			/**
			 * Split a string by whitespace
			 *
			 * @return A vector of strings, each of which is a substring of the string
			 */
			vector<ext_string> split(size_type limit = npos) const
			{
				vector<ext_string> v;

				const_iterator 
					i = begin(),
					  last = i;
				for (; i != end(); i++)
				{
					if (*i == ' ' || *i == '\n' || *i == '\t' || *i == '\r')
					{
						if (i + 1 != end() && (i[1] == ' ' || i[1] == '\n' || i[1] == '\t' || i[1] == '\r'))
							continue;
						v.push_back(ext_string(last, i));
						last = i + 1;
						if (v.size() >= limit - 1)
						{
							v.push_back(ext_string(last, end()));
							return v;
						}
					}
				}

				if (last != i)
					v.push_back(ext_string(last, i));

				return v;
			}

			/**
			 * Split a string by a character
			 *
			 * Returns a vector of ext_strings, each of which is a substring of the string formed by splitting
			 * it on boundaries formed by the character @p separator.  If @p limit is set, the returned vector
			 * will contain a maximum of @p limit elements with the last element containing the rest of
			 * the string.
			 *
			 * If @p separator is not found in the string, a single element will be returned in the vector
			 * containing the entire string.
			 *
			 * The separators are removed from the output
			 *
			 * @param[in] separator The character separator to split the string on
			 * @param[in] limit     The maximum number of output elements
			 * @return A vector of strings, each of which is a substring of the string
			 *
			 * @section split_ex Example
			 * @code
			 * std::ext_string s("This|is|a|test.");
			 * std::vector<std::ext_string> v = s.split('|');
			 * std::copy(v.begin(), v.end(), std::ostream_iterator<std::ext_string>(std::cout, "\n"));
			 *
			 * This
			 * is
			 * a
			 * test.
			 * @endcode
			 */
			vector<ext_string> split(value_type separator, size_type limit = npos) const
			{
				vector<ext_string> v;

				const_iterator 
					i = begin(),
					last = i;
				for (; i != end(); i++)
				{
					if (*i == separator)
					{
						v.push_back(ext_string(last, i));
						last = i + 1;
						if (v.size() >= limit - 1)
						{
							v.push_back(ext_string(last, end()));
							return v;
						}
					}
				}

				if (last != i)
					v.push_back(ext_string(last, i));

				return v;
			}

			/**
			 * Split a string by another string
			 *
			 * Returns a vector of ext_strings, each of which is a substring of the string formed by
			 * splitting it on boundaries formed by the string @p separator.  If @p limit is set, the
			 * returned vector will contain a maximum of @p limit elements with the last element
			 * containing the rest of the string.
			 *
			 * If @p separator is not found in the string, a single element will be returned in the
			 * vector containing the entire string.
			 *
			 * The separators are removed from the output
			 *
			 * @param[in] separator The string separator to split the string on
			 * @param[in] limit     The maximum number of output elements
			 * @return A vector of strings, each of which is a substring of the string
			 *
			 * @ref split_ex
			 */
			vector<ext_string> split(const string &separator, size_type limit = npos) const
			{
				vector<ext_string> v;

				const_iterator
					i = begin(),
					last = i;
				for (; i != end(); i++)
				{
					if (string(i, i + separator.length()) == separator)
					{
						v.push_back(ext_string(last, i));
						last = i + separator.length();

						if (v.size() >= limit - 1)
						{
							v.push_back(ext_string(last, end()));
							return v;
						}
					}
				}

				if (last != i)
					v.push_back(ext_string(last, i));

				return v;
			}

			/**
			 * Convert a string into an integer
			 *
			 * Convert the initial portion of a string into a signed integer.  Once a non-numeric
			 * character is reached, the remainder of @p string is ignored and the integer that was
			 * read returned.
			 *
			 * @param s The string to convert
			 * @return The integer converted from @p string
			 */
			static long int integer(const string &s)
			{
				long int retval = 0;
				bool neg = false;

				for (const_iterator i = s.begin(); i != s.end(); i++)
				{
					if (i == s.begin())
					{
						if (*i == '-')
						{
							neg = true;
							continue;
						}
						else if (*i == '+')
							continue;
					}
					if (*i >= '0' && *i <= '9')
					{
						retval *= 10;
						retval += *i - '0';
					}
					else
						break;
				}

				if (neg)
					retval *= -1;

				return retval;
			}

			/**
			 * Convert the string to an integer
			 *
			 * Convert the initial portion of the string into a signed integer.  Once a non-numeric
			 * character is reached, the remainder of the string is ignored and the integer that had
			 * been read thus far is returned.
			 *
			 * @return The integer converted from the string
			 */
			long int integer() const
			{
				return integer(*this);
			}

			/**
			 * Split a string into chunks of size @p chunklen.  Returns a vector of strings.
			 *
			 * Splits a string into chunks of the given size.  The final chunk may not fill its
			 * entire allocated number of characters.
			 *
			 * @param[in] chunklen The number of characters per chunk
			 * @return A vector of strings, each of length <= chunklen
			 *
			 * @section chunk_split-ex Example
			 * @code
			 * std::ext_string s("abcdefghijk");
			 * std::vector<std::ext_string> v = s.chunk_split(3);
			 * std::copy(v.begin(), v.end(), ostream_iterator<std::ext_string>(cout, " "));
			 *
			 * abc def ghi jk
			 * @endcode
			 */
			vector<ext_string> chunk_split(size_type chunklen) const
			{
				vector<ext_string> retval;
				retval.reserve(size() / chunklen + 1);

				size_type count = 0;
				const_iterator
					i = begin(),
					last = i;
				for (; i != end(); i++, count++)
				{
					if (count == chunklen)
					{
						count = 0;
						retval.push_back(ext_string(last, i));
						last = i;
					}
				}
				
				if (last != i)
					retval.push_back(ext_string(last, i));

				return retval;
			}

			/**
			 * Join a sequence of strings by some glue to create a new string
			 *
			 * Glue is not added to the end of the string.
			 *
			 * @pre [first, last) is a valid range
			 * @pre InputIterator is a model of STL's Input Iterator
			 * @pre InputIterator must point to a string type (std::string, std::ext_string, char *)
			 *
			 * @param[in] glue  The glue to join strings with
			 * @param[in] first The beginning of the range to join
			 * @param[in] last  The end of the range to join
			 * @return A string constructed of each element of the range connected together with @p glue
			 *
			 * @section join_ex Example
			 * @code
			 * std::vector<std::ext_string> v;
			 * v.push_back("This");
			 * v.push_back("is");
			 * v.push_back("a");
			 * v.push_back("test.");
			 * std::cout << std::ext_string::join("|", v.begin(), v.end()) << std::endl;
			 *
			 * This|is|a|test.
			 * @endcode
			 */
			template <class InputIterator>
				static ext_string join(const string &glue, InputIterator first, InputIterator last)
				{
					ext_string retval;

					for (; first != last; first++)
					{
						retval.append(*first);
						retval.append(glue);
					}
					retval.erase(retval.length() - glue.length());

					return retval;
				}

			/**
			 * Join a sequence of strings by some glue to create a new string
			 *
			 * @copydoc join
			 * @ref join_ex
			 */
			template <class InputIterator>
				static ext_string join(value_type glue, InputIterator first, InputIterator last)
				{
					ext_string retval;

					for (; first != last; first++)
					{
						retval.append(*first);
						retval.append(1, glue);
					}
					retval.erase(retval.length() - 1);

					return retval;
				}

			/**
			 * Search for any instances of @p needle and replace them with @p s
			 *
			 * @param[in] needle The string to replace
			 * @param[in] s      The replacement string
			 * @return				*this
			 * @post					All instances of @p needle in the string are replaced with @p s
			 *
			 * @section replace-ex Example
			 * @code
			 * std::ext_string s("This is a test.");
			 * s.replace("is", "ere");
			 * std::cout << s << std::endl;
			 *
			 * There ere a test.
			 * @endcode
			 */
			ext_string &replace(const string &needle, const string &s)
			{
				size_type
					lastpos = 0,
					thispos;

				while ((thispos = find(needle, lastpos)) != npos)
				{
					string::replace(thispos, needle.length(), s);
					lastpos = thispos + 1;
				}
				return *this;
			}

			/**
			 * Search of any instances of @p needle and replace them with @p c
			 *
			 * @param[in] needle The character to replace
			 * @param[in] c      The replacement character
			 * @return           *this
			 * @post             All instances of @p needle in the string are replaced with @p c
			 *
			 * @ref replace-ex
			 */
			ext_string &replace(value_type needle, value_type c)
			{
				for (iterator i = begin(); i != end(); i++)
					if (*i == needle)
						*i = c;

				return *this;
			}

			/**
			 * Repeat a string @p n times
			 *
			 * @param[in] n The number of times to repeat the string
			 * @return ext_string containing @p n copies of the string
			 *
			 * @section repeat-ex Example
			 * @code
			 * std::ext_string s("123");
			 * s = s * 3;
			 * std::cout << s << std::endl;
			 *
			 * 123123123
			 * @endcode
			 */
			ext_string operator*(size_type n)
			{
				ext_string retval;
				for (size_type i = 0; i < n; i++)
					retval.append(*this);

				return retval;
			}

			/**
			 * Convert the string to lowercase
			 *
			 * @return *this
			 * @post The string is converted to lowercase
			 */
			ext_string &tolower()
			{
				for (iterator i = begin(); i != end(); i++)
					if (*i >= 'A' && *i <= 'Z')
						*i = (*i) + ('a' - 'A');
				return *this;
			}

			/**
			 * Convert the string to uppercase
			 *
			 * @return *this
			 * @post The string is converted to uppercase
			 */
			ext_string &toupper()
			{
				for (iterator i = begin(); i != end(); i++)
					if (*i >= 'a' && *i <= 'z')
						*i = (*i) - ('a' - 'A');
				return *this;
			}

			/**
			 * Count the occurances of @p str in the string.
			 *
			 * @return The count of substrings @p str in the string
			 */
			size_type count(const string &str) const
			{
				size_type
					count = 0,
					last = 0,
					cur = 0;

				while ((cur = find(str, last + 1)) != npos)
				{
					count++;
					last = cur;
				}

				return count;
			}

			/**
			 * Determine if the string is alphanumeric
			 *
			 * @return true if the string contains only characters between a-z, A-Z and 0-9 and
			 * contains at least one character, else false
			 */
			bool is_alnum() const
			{
				if (length() == 0)
					return false;

				for (const_iterator i = begin(); i != end(); i++)
				{
					if (*i < 'A' || *i > 'Z')
						if (*i < '0' || *i > '9')
							if (*i < 'a' || *i > 'z')
								return false;
				}

				return true;
			}

			/**
			 * Determine if the string is alphabetic only
			 *
			 * @return true of the string contains only characters between a-z and A-Z and contains at
			 * least one character, else false
			 */
			bool is_alpha() const
			{
				if (length() == 0)
					return false;

				for (const_iterator i = begin(); i != end(); i++)
					if (*i < 'A' || (*i > 'Z' && (*i < 'a' || *i > 'z')))
						return false;

				return true;
			}

			/**
			 * Determine if the string is numeric only
			 *
			 * @return true if the string contains only characters between 0-9 and contains at least
			 * one character, else false
			 */
			bool is_numeric() const
			{
				if (length() == 0)
					return false;

				for (const_iterator i = begin(); i != end(); i++)
					if (*i < '0' || *i > '9')
						return false;

				return true;
			}

			/**
			 * Determine if a string is all lower case
			 *
			 * @return true if there is at least one character, and all characters are lowercase
			 * letters, else false
			 */
			bool is_lower() const
			{
				if (length() == 0)
					return false;

				for (const_iterator i = begin(); i != end(); i++)
					if (*i < 'a' || *i < 'z')
						return false;

				return true;
			}

			/**
			 * Determine if a string is all upper case
			 *
			 * @return true if there is at least one character, and all characters are uppercase
			 * letters, else false
			 */
			bool is_upper() const
			{
				if (length() == 0)
					return false;

				for (const_iterator i = begin(); i != end(); i++)
					if (*i < 'A' || *i > 'Z')
						return false;

				return true;
			}

			/**
			 * Swap the case of a string
			 *
			 * @post Converts all uppercase to lowercase, and all lowercase to uppercase in the string
			 * @return *this
			 */
			ext_string &swapcase()
			{
				for (iterator i = begin(); i != end(); i++)
					if (*i >= 'A' && *i <= 'Z')
						*i += ('a' - 'A');
					else if (*i >= 'a' && *i <= 'z')
						*i -= ('a' - 'A');
				
				return *this;
			}
	};
}
#endif
