/****************************************************************************
//
// \file
//      Defines routines for I/O of Frame and related objects.
// \verbatim
//      Spaces, tabs and newlines do not have any importance.
//      Comments are allowed C-style,C++-style, make/perl/csh -style
//      Description of the I/O :
//        Vector  : OUTPUT : e.g. [10,20,30] 
//                  INPUT  :
//                         1) [10,20,30]
//                         2) Zero
//        Twist   : e.g. [1,2,3,4,5,6] 
//           where [1,2,3] is velocity vector
//           where [4,5,6] is rotational velocity vector
//        Wrench  : e.g. [1,2,3,4,5,6]
//           where [1,2,3] represents a force vector
//           where [4,5,6] represents a torque vector
//        Rotation : output : 
//                 [1,2,3;
//                  4,5,6;
//                  7,8,9] cfr definition of Rotation object.
//                  input :
//                    1) like the output
//                    2) EulerZYX,EulerZYZ,RPY word followed by a vector, e.g. :
//                        Eulerzyx[10,20,30]
//                   (ANGLES are always expressed in DEGREES for I/O)
//                   (ANGELS are always expressed in RADIANS for internal representation)
//                    3) Rot [1,2,3] [20]  Rotates around axis [1,2,3] with an angle
//                   of 20 degrees.
//                    4) Identity          returns identity rotation matrix.
//       Frames   : output : [ Rotationmatrix positionvector ]
//                   e.g. [ [1,0,0;0,1,0;0,0,1] [1,2,3] ]
//                  Input  : 
//                     1) [ Rotationmatrix positionvector ]
//                     2) DH [ 10,10,50,30]  Denavit-Hartenberg representation
//                     ( is in fact not the representation of a Frame, but more
//                       limited, cfr. documentation of Frame object.)
//  \endverbatim
//
// \warning
//   You can use <iostream.h> or <iostream> header files for file I/O, 
//   if one declares the define WANT_STD_IOSTREAM then the standard C++
//   iostreams headers are included instead of the compiler-dependent version   
//
 *       
 *  \author 
 *      Erwin Aertbelien, Div. PMA, Dep. of Mech. Eng., K.U.Leuven
 *
 *  \version 
 *      ORO_Geometry V0.2
 *
 *  \par History
 *      - $log$
 *
 *  \par Release
 *      $Id: frames_io.h,v 1.1.1.1.2.5 2003/06/26 15:23:59 psoetens Exp $
 *      $Name:  $ 
 ****************************************************************************/
#ifndef FRAMES_IO_H
#define FRAMES_IO_H

#include "utility.h"

#include <pkgconf/os.h>
#if OROINT_OS_STDIOSTREAM

#include "utility_io.h"
#include "frames.h"

#ifdef USE_NAMESPACE
namespace ORO_Geometry {
#endif

//! width to be used when printing variables out with frames_io.h
//! global variable, can be changed.
//! \TODO
//! This global variable is not thread-safe, should be thread-local storage.
extern int FRAMEWIDTH;


using namespace std;



// I/O to C++ stream.
ostream& operator << (ostream& os,const Vector& v);
ostream& operator << (ostream& os,const Rotation& R);
ostream& operator << (ostream& os,const Frame& T);
ostream& operator << (ostream& os,const Twist& T);
ostream& operator << (ostream& os,const Wrench& T);
ostream& operator << (ostream& os,const Vector2& v);
ostream& operator << (ostream& os,const Rotation2& R);
ostream& operator << (ostream& os,const Frame2& T);


istream& operator >> (istream& is,Vector& v);
istream& operator >> (istream& is,Rotation& R);
istream& operator >> (istream& is,Frame& T);
istream& operator >> (istream& os,Twist& T);
istream& operator >> (istream& os,Wrench& T);
istream& operator >> (istream& is,Vector2& v);
istream& operator >> (istream& is,Rotation2& R);
istream& operator >> (istream& is,Frame2& T);

    
#ifdef USE_NAMESPACE
} // namespace Frame
#endif

#endif // OROINT_OS_STDIOSTREAM

#endif
