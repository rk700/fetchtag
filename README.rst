====================
README for fetchtag
====================

fetchtag can automatically update tags of audio files by searching the Internet. It is written in C, and Lua scripts are used as extensions for searching on different sites. 

Currently it only supports a Chinese music website(music.douban.com), but some other sites will be added in the future. You can also write your own Lua scripts for any sites you like.


Assignment
----------

fetchtag will look into the directory, and compute `edit distances <en.wikipedia.org/wiki/Edit_distance>`_ for all pairs of music files and track titles. Then `Munkres Algorithm <en.wikipedia.org/wiki/Hungarian_algorithm>`_ is used for assigning tracks to music files.

Recover
-------

By default, fetchtag will backup tag information before updating. So if you can restore if something's wrong in updating.


Dependencies
------------

fetchtag uses `Taglib <taglib.github.com>`_ for editing audio file tags. 

You should also have `Lua <www.lua.org>`_ installed for running extensions.


Comments and bug reports
------------------------
Project page is on
https://github.com/rk700/fetchtag

You can also send email to the author:
`Ruikai Liu`_ 

.. _Ruikai Liu: lrk700@gmail.com
