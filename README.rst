Notice
=======
This repo is a very early experiment of my idea, and is not active now because lack of participation and most importantly, author decides to focus on more practical project (this project relies on Email server, which is only practical in a small scale deployment). If you still think this is useful, send an issue to me and I would also like to help as always.

INTRO:                             
======
  tknet is designed to become a simple cross-platform P2P library.

  tknet uses traditional centralized-server-based model, however, different from other P2P solutions, tknet peer is able to use Email server to track center servers (namely bridge peers or BDG peers) when necessary. It uses a private protocol and is still under development. Despite these, it works and it is simple.

  I simply include how to build it in this README and have not written documents before I think it is at least usable and easy-to-use. If you are interested in this project, please let me know! I need some incentive to let tknet go further.

BUILD:
======
STEP 1: 
~~~~~~
Scons(see http://www.scons.org/) is required for building, OpenSSL-dev library is also needed if you want to enable SSL feature.

STEP 2: 
~~~~~~
Run the './tknet.sh build' command in the source directory, an executable demo program and the tknet binary library are created under ./bin folder.

LICENSE:
========
tknet source code is used under the terms of the GNU General Public License version 3.0 as published by the Free Software Foundation and appearing in the file LICENSE.GPL included in the packaging of this file.  Please review the following information to ensure the GNU General Public License version 3.0 requirements will be met: 

http://www.gnu.org/copyleft/gpl.html

CONTACT:
========
Feel free to contact me via clock126@126.com.
