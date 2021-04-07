BaseZ
======
1. Description
2. License
3. Compile and Install Guide
4. Build, Install deb Package
5. Build, Install rpm Package
6. Quick Start Usage Guide


DESCRIPTION
===========
Encode  data into/decode data from base16, base32, base32hex, base64 or
base64url stream per RFC 4648;  MIME  base64  Content-Transfer-Encoding
per RFC 2045; or PEM Printable Encoding per RFC 1421.


LICENSE
=======
The BaseZ package: GPLv3+
Documentation files: GPLv3+ or CC-BY-SA-3.0+
Core files: 2-clause BSD

For more details see the LICENSE file included in the package.


COMPILE AND INSTALL GUIDE
=========================
Use the standard building tools freely available in Linux and BSD
distributions, gratis "Command Line Tools for Xcode" on Mac, or free Mingw
with MSYS on MS Windows.

Unpack the basez-<version>.tar.gz source package in a src directory and 'cd'
into it.

For building configuration options type:

  ./configure --help

To compile the software, type:

  ./configure
  make

To install, type:

  make install
or
  sudo make install

To uninstall, cd into the src/basez-x.x directory and type:

  make uninstall
or
  sudo make uninstall


BUILD, INSTALL DEB PACKAGE
==========================
Install the standard development and package building tools distributed with
Debian, Ubuntu or a Debian-based distribution.

Unpack the basez-<version>.tar.gz source package in a src directory and 'cd'
into it.

Build the package:

  ./configure
  make debfiles
  debuild -us -uc -b

Install the package:

  dpkg -i basez_<version>_<arch>.deb


BUILD, INSTALL RPM PACKAGE
==========================
Set up the standard development and package building tools distributed with
Red Hat, CentOS, Fedora or a RPM-based distribution.

Build the package:

  rpmbuild -tb basez-<version>.tar.gz

Install the package:

  rpm -Uvh basez-<version>-<arch>.rpm


QUICK START USAGE GUIDE
=======================
For list of program options, install the software and type:

  basez --help
or
  man basez


Copyright (C) 2013, 2015, 2016  Milan Kupcevic
==============================================
Copying and distribution of this file are permitted under the terms of the
GNU General Public License version 3, or any later version, or alternatively
under the terms of the Creative Commons Attribution-ShareAlike 3.0 license,
or a later version.
