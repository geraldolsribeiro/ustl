// This file is part of the ustl library, an STL implementation.
// Copyright (C) 2003 by Mike Sharov <msharov@talentg.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the 
// Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
// Boston, MA  02111-1307  USA.
//
// ustdxept.h
//

#ifndef USTDXEPT_H_46F7AE967738B588038F95E41158D7FF
#define USTDXEPT_H_46F7AE967738B588038F95E41158D7FF

#include "uexception.h"
#include "ustring.h"

namespace ustl {

static const xfmt_t	xfmt_LogicError		= 2;
static const xfmt_t	xfmt_RuntimeError	= 3;

/// \class logic_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Logic errors represent problems in the internal logic of the program.
///
class logic_error : public exception {
public:
    explicit		logic_error (const string& arg) throw();
    inline virtual     ~logic_error (void) throw() {}
    virtual const char*	what (void) const throw();
    virtual void	info (string& msgbuf, const char* fmt = NULL) const throw();
    virtual void	read (istream& is);
    virtual void	write (ostream& os) const;
    virtual size_t	stream_size (void) const;
protected:
    string		m_Arg;
};

/// \class domain_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports domain errors ("domain" is in the mathematical sense)
///
class domain_error : public logic_error {
public:
    explicit		domain_error (const string& arg) throw();
    virtual const char*	what (void) const throw();
};

/// \class invalid_argument ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports an invalid argument to a function.
///
class invalid_argument : public logic_error {
public:
    explicit		invalid_argument (const string& arg) throw();
    virtual const char*	what (void) const throw();
};

/// \class length_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports when an object exceeds its allowed size.
///
class length_error : public logic_error {
public:
    explicit		length_error (const string& arg) throw();
    virtual const char*	what (void) const throw();
};

/// \class out_of_range ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports arguments with values out of allowed range.
///
class out_of_range : public logic_error {
public:
    explicit		out_of_range (const string& arg) throw();
    virtual const char*	what (void) const throw();
};

/// \class runtime_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports errors that are dependent on the data being processed.
///
class runtime_error : public exception {
public:
    explicit		runtime_error (const string& arg) throw();
    virtual	       ~runtime_error (void) throw();
    virtual const char*	what (void) const throw();
    virtual void	info (string& msgbuf, const char* fmt = NULL) const throw();
    virtual void	read (istream& is);
    virtual void	write (ostream& os) const;
    virtual size_t	stream_size (void) const;
protected:
    string		m_Arg;
};

/// \class range_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports data that does not fall within the permitted range.
///
class range_error : public runtime_error {
public:
    explicit		range_error (const string& arg) throw();
    virtual const char*	what (void) const throw();
};

/// \class overflow_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports arithmetic overflow.
///
class overflow_error : public runtime_error {
public:
    explicit		overflow_error (const string& arg) throw();
    virtual const char*	what (void) const throw();
};

/// \class underflow_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports arithmetic underflow.
///
class underflow_error : public runtime_error {
public:
    explicit		underflow_error (const string& arg) throw();
    virtual const char*	what (void) const throw();
};

}; // namespace ustl

#endif
