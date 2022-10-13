@echo off

Set PATH=H:\c2cbuild\Vc7\bin;H:\c2cbuild\Microsoft SDKs\Windows\v6.0\bin;C:\Program Files (x86)\Intel\Compiler\11.1\060\bin\ia32;%PATH%
Set INCLUDE=H:\c2cbuild\Vc7\include;H:\c2cbuild\Microsoft SDKs\Windows\v6.0\include;C:\Program Files (x86)\Intel\Compiler\11.1\060\include;C:\Program Files (x86)\Intel\Compiler\11.1\060\include\ia32;%INCLUDE%
Set LIB=H:\c2cbuild\Vc7\lib;H:\c2cbuild\Microsoft SDKs\Windows\v6.0\lib;C:\Program Files (x86)\Intel\Compiler\11.1\060\lib\ia32;%LIB%
#Set PATH=C:\Program Files (x86)\Microsoft Visual C++ Toolkit 2003\bin;C:\Program Files (x86)\Windows Kits\8.0\bin\x86;C:\Program Files (x86)\Intel\Composer XE 2011 SP1\bin\ia32;%PATH%
#Set INCLUDE=C:\Program Files (x86)\Intel\Composer XE 2011 SP1\compiler\include;C:\Program Files (x86)\Microsoft Visual C++ Toolkit 2003\include;C:\Program Files (x86)\Windows Kits\8.0\include\shared;C:\Program Files (x86)\Windows Kits\8.0\include\um;C:\Program Files (x86)\Intel\Composer XE 2011 SP1\compiler\include\ia32;%INCLUDE%
#Set LIB=C:\Program Files (x86)\Microsoft Visual C++ Toolkit 2003\lib;C:\Program Files (x86)\Windows Kits\8.0\lib\win8\um\x86;C:\Program Files (x86)\Intel\Composer XE 2011 SP1\compiler\lib\ia32;%LIB%


echo Setting environment for using Microsoft Visual C++ 2003 Toolkit.
echo (If you have another version of Visual Studio or Visual C++ installed and wish
echo to use its tools from the command line, run vcvars32.bat for that version.)
echo.
echo Thank you for choosing the Visual C++ Toolkit 2003!  Get started quickly by
echo building the code samples included in the "Samples" directory.  Each sample
echo includes a short whitepaper discussing the Visual C++ features, and a batch
echo file for building the code.
echo.
echo Type "cl /?" for brief documentation on compiler options.
echo.
echo Visit http://msdn.microsoft.com/visualc/using/documentation/default.aspx for
echo complete compiler documentation.
