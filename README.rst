 .. image:: http://www.thoughts-of.me/twbook/images/tknet_link.png
INTRO:                             
======
tknet is designed to become a simple cross-platform P2P library.

tknet uses traditional centralized server-based model, however, different from other P2P solutions, tknet is able to use E-mail server to track center servers (namely bridge peer or BDG peer) lest others are closed down; it was written from scratch, using a private protocol, simple as it is and still under development; despite this, now it is able to establish connections between peers in a foolproof way. I simply include how to build it and not writing documents before I think it is almost usable and easy-to-use.

BUILD:
======
STEP 1: 
~~~~~~
Scons(see http://www.scons.org/) is required for building, OpenSSL-dev library is also required if you want to enable the SSL feature.

STEP 2: 
~~~~~~
Run the './tknet.sh build' command in the source directory, an executable demo program and tknet binary library are created under ./bin folder.

NOTE: 
=====
  1: The './tknet.sh win' help you generate the DOS format source codes in a new folder.

  2: Use 'help' command in demo stdin to see built-in commands.

LICENSE:
========
tknet source code is used under the terms of the GNU General Public License version 3.0 as published by the Free Software Foundation and appearing in the file LICENSE.GPL included in the packaging of this file.  Please review the following information to ensure the GNU General Public License version 3.0 requirements will be met: 

http://www.gnu.org/copyleft/gpl.html

CONTACT:
========
You can contact me via clock126@126.com.
