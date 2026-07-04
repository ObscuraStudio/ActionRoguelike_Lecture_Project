The dlib code is Release 19.23 from https://github.com/davisking/dlib

The following diff was applied to image_transforms/morphological_operations.h to correct a macro clash:

854c854
<         auto check = [&](long r, long c)
---
>         auto localCheck = [&](long r, long c) //@UE5 changed from just "check" to avoid a clash when used in UE
864,871c864,871
<         check(r-1,c-1);
<         check(r-1,c);
<         check(r-1,c+1);
<         check(r,c+1);
<         check(r+1,c+1);
<         check(r+1,c);
<         check(r+1,c-1);
<         check(r,c-1);
---
>         localCheck(r-1,c-1);
>         localCheck(r-1,c);
>         localCheck(r-1,c+1);
>         localCheck(r,c+1);
>         localCheck(r+1,c+1);
>         localCheck(r+1,c);
>         localCheck(r+1,c-1);
>         localCheck(r,c-1);

The following code has been added to dlib/Include/dlib/dnn/core.h
#if defined(_MSC_VER) && !defined(__clang__) instead of #ifdef _MSC_VER (line 23)
This is to fix the clang build in UE5

The following change has been applied to both dlib/Include/dlib/unicode/unicode.h and dlib/Source/dlib/unicode/unicode.h:
The #if guard on the unichar_traits struct has been extended from:
  #if defined(__GNUC__) && __GNUC__ < 4 && __GNUC_MINOR__ < 4
to:
  #if (defined(__GNUC__) && __GNUC__ < 4 && __GNUC_MINOR__ < 4) || defined(_LIBCPP_VERSION)
This activates the explicit unichar_traits specialisation when building with libc++ (Linux and Mac),
which does not provide std::char_traits for unsigned integer types, causing a hard error in clang 20.